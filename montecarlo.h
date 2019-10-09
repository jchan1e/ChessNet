#ifndef MONTECARO_H
#define MONTECARLO_H

#include "Engine/engine.h"

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
      mtx.lock();
      float explore = C*(sqrt(log(parent->visits)/visits));
      float exploit = 0.0;
      if (visits != 0) {
        if (!opponent)
          exploit = wins/visits;
        else
          exploit = 1.0 - wins/visits;
      }
      mtx.unlock();
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
  public:
    MonteCarloTree(bool white, float c) {
      Ce = c;
      Action A = {0};
      White = white;
      root = new Node(NULL, A, E.getBoardState());
      root->opponent = White;
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
      BoardState S = BS;
      vector<Action> A_list;
      while (E.getLegalMoves(&A_list, S) > 0) { // && BS.winner == -1) {
        int R = rand() % A_list.size();
        S = E.advance(S, A_list[R]);
      }
      if (S.winner == -1) // || S.winner == 0.5)
        return 0.5;
      return S.winner;
    }

    void Run(float* wins, int* visits, Action* A, bool* stop) {
      int threadCount = thread::hardware_concurrency();
      //int threadCount = 2;
      vector<thread> threads;
      for (int i=0; i < threadCount; ++i) {
        thread th(&MonteCarloTree::run, this, stop);
        threads.push_back(move(th));
      }
      for (thread &th : threads) {
        th.join();
      }
      getBestChoice(wins, visits, A);
    }

    void run(bool* stop) {
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
        if (N->UCB(1) > node->UCB(1))
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
