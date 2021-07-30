#ifndef MONTECARO_H
#define MONTECARLO_H

#include "Engine/engine.h"
#include "NN/neuralnet.h"

#include <cstdlib>
#include <cmath>
#include <vector>
#include <thread>
#include <mutex>

#include <iostream>

class Node {
  public:
    Node* parent = NULL;
    vector<Node*> children;
    float wins = 0.0;
    int visits = 0;
    bool expanded = false;
    bool endstate = false;
    bool opponent;
    Action action;
    BoardState state;

    mutex mtx;

    Node(Node* P, Action A, BoardState S) {
      mtx.lock();
      parent = P;
      action = A;
      state = S;
      if (P != NULL)
        opponent = !parent->opponent;
      mtx.unlock();
    }

    ~Node() {
      mtx.lock();
      for (Node* child : children) {
        delete child;
      }
      mtx.unlock();
    }

    float UCB(float C) {
      // Upper Confidence Bound
      // Lower C favors exploitation, higher favors exploration
      //mtx.lock();
      float explore = C*(sqrt(log(parent->visits)/visits));
      float exploit = 0.0;
      if (visits != 0) {
        if (!opponent)
          exploit = wins/visits;
        else
          exploit = 1.0 - wins/visits;
      }
      //mtx.unlock();
      return exploit + explore;
    }

    //bool operator<(const Node& b) {
    //  return UCB(sqrt(2)) < b.UCB(sqrt(2));
    //}

};

class MonteCarloTree {
  public:
    Node* root;
    bool White;
    Engine E;
    float Ce; // exploration bias parameter
    Neuralnet* N;
  public:
    MonteCarloTree(bool white, float c, char* Agentfile) {
      Ce = c;
      Action A = {0};
      White = white;
      root = new Node(NULL, A, E.getBoardState());
      root->opponent = White;
      N = new Neuralnet(Agentfile);
    }

    ~MonteCarloTree() {
      delete root;
      delete N;
    }

    void backpropagate(Node* N, float score) {
      while (N->parent != NULL) {
        N->mtx.lock();
        N->wins += score;
        //N->visits++;
        N->mtx.unlock();
        N = N->parent;
      }
      root->mtx.lock();
      //root->visits++;
      root->wins += score;
      root->mtx.unlock();
    }

    void expand(Node* N) {
      N->mtx.lock();
      if (!N->expanded) {
        N->expanded = true;
        if (N->parent != NULL && N->parent->parent != NULL && N->parent->parent->parent != NULL &&  N->parent->parent->parent->parent != NULL &&
            N->state == N->parent->parent->parent->parent->state) {
          N->endstate = true;
          N->state.winner = 0.5;
        }
        else {
          vector<Action> A_list;
          int moves = E.getLegalMoves(&A_list, N->state);
          if (moves > 0) {
            for (int i=0; i < moves; ++i){
              Node* n = new Node(N, A_list[i], E.advance(N->state, A_list[i]));
              N->children.push_back(n);
            }
          }
          else
            N->endstate = true;
        }
      }
      N->mtx.unlock();
    }

    float rollout(BoardState BS) {
      // transcribe boardstate to NN input array and evaluate
      float BS_array[12*64+5] = {0.0};
      float result[2];
      for (int i=0; i < 8; ++i) {
        for (int j=0; j < 8; ++j) {
          if (BS.board[i][j] != blank) {
            int l;
            switch (BS.board[i][j]) {
              case w_pawn:
                l = 0;
                break;
              case w_bishop:
                l = 1;
                break;
              case w_knight:
                l = 2;
                break;
              case w_rook:
                l = 3;
                break;
              case w_queen:
                l = 4;
                break;
              case w_king:
                l = 5;
                break;
              case b_pawn:
                l = 6;
                break;
              case b_bishop:
                l = 7;
                break;
              case b_knight:
                l = 8;
                break;
              case b_rook:
                l = 9;
                break;
              case b_queen:
                l = 10;
                break;
              case b_king:
                l = 11;
                break;
              default:
                l = 0; //shouldn't happen but just in case
            }
            BS_array[64*l + 8*i + j] = 1.0;
          }
        }
      }
      BS_array[12*64 + 0] = -1.0 + 2.0*(BS.turn%2);
      BS_array[12*64 + 1] = BS.w_castle_kingside?1.0:0.0;
      BS_array[12*64 + 2] = BS.w_castle_queenside?1.0:0.0;
      BS_array[12*64 + 3] = BS.b_castle_kingside?1.0:0.0;
      BS_array[12*64 + 4] = BS.b_castle_queenside?1.0:0.0;

      N->eval((float*)&BS_array, (float*)&result);
      return result[0] / (result[0]+result[1]);

      //BoardState S = BS;
      //vector<Action> A_list;
      //while (E.getLegalMoves(&A_list, S) > 0) { // && BS.winner == -1) {
      //  int R = rand() % A_list.size();
      //  S = E.advance(S, A_list[R]);
      //}
      //if (S.winner == -1) // || S.winner == 0.5)
      //  return 0.5;
      //return S.winner;
    }

    void Run(float* wins, int* visits, Action* A, bool* stop) {
      //int threadCount = min(6u,thread::hardware_concurrency());
      int threadCount = 2;
      vector<thread> threads;
      for (int i=0; i < threadCount; ++i) {
        thread th(&MonteCarloTree::run_thread, this, stop);
        threads.push_back(move(th));
      }
      for (thread &th : threads) {
        th.join();
      }
      getBestChoice(wins, visits, A);
    }

    void run_thread(bool* stop) {
      while (*stop == false) { // wait for separate thread to end evaluation
        Node* N = root;
        //traverse the tree by highest UCB until we reach a leaf node
        while (!N->children.empty()) {
          N->mtx.lock();
          N->visits++;
          int best_i = 0;
          float best_UCB = N->children[0]->UCB(Ce);
          for (unsigned int i=1; i < N->children.size(); ++i) {
            float i_ucb = N->children[i]->UCB(Ce);
            if (i_ucb > best_UCB) {
              best_i = i;
              best_UCB = i_ucb;
            }
          }
          N->mtx.unlock();
          N = N->children[best_i];
        }
        N->mtx.lock();
        N->visits++;
        N->mtx.unlock();

        //E.printBoard(N->state);

        float win;
        if (N->visits == 1) {
          // if node is unvisited, run rollout and backpropagate
          win = rollout(N->state);
        }
        else if (N->endstate) {
          // if endstate, backpropagate score stored in game state
          win = 0.5;
          if (N->state.winner != -1)
            win = N->state.winner;
        }
        else {
          // otherwise, expand leaf node and pick a child node to rollout
          expand(N);
          if (!N->children.empty()) {
            int r = rand() % N->children.size();
            N = N->children[r];
          }
          win = rollout(N->state);
        }
        if (!White)
          win = 1.0 - win;
        backpropagate(N, win);
      }
    }

    void getBestChoice(float* wins, int* visits, Action* A) {
      // return list of best actions
      Node* node = root;
      if (!node->children.empty())
        node = root->children[0];
      for (Node* N : root->children) {
        if (N->visits > node->visits)
          node = N;
      }
      *A = node->action;
      *wins = node->wins;
      *visits = node->visits;
    }

    bool advance(Action A) {
      Node* n = NULL;
      for (Node* N : root->children) {
        if (N->action == A)
          n = N;
      }
      if (!n) {
        vector<Action> A_list;
        E.getLegalMoves(&A_list, root->state);
        bool valid = false;
        for (Action B : A_list) {
          if (A == B)
            valid = true;
        }
        if (valid) {
          Node* child = new Node(root, A, E.advance(root->state, A));
          root->children.push_back(child);
          n = child;
        }
        else {
          cout << "error: move not possible\n";
          return false;
        }
      }
      E.advance(A);
      advance(n);
      return true;
    }

    void advance(Node* R) {
      //// Recursively advance to parent node first
      //if (R->parent != root)
      //  advance(R->parent);

      // Delete all other choices
      for (Node* N : root->children) {
        if (N != R)
          delete N; // implicitly delete grandchildren
      }
      root->children.clear();
      delete root;
      root = R;
      root->parent = NULL;
    }

    //float UCB(Node n) {
    //  float v = n->score;
    //  if (n->parent == NULL)
    //    return v;
    //  return v + C*(math.sqrt(math.ln(n.parent->visits)/n.visits))
    //}

    vector<Action> getMoveList() {
      return E.getHist();
    }

};

#endif
