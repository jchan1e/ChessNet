#ifndef MONTECARO_H
#define MONTECARLO_H

#include "Engine/engine.h"

#include <cstdlib>
#include <cmath>
#include <vector>

#include <iostream>

class Node {
  public:
    Node* parent = NULL;
    vector<Node*> children;
    float wins = 0.0;
    int visits = 0;
    bool endstate = false;
    bool opponent;
    Action action;
    BoardState state;

    Node(Node* P, Action A, BoardState S) {
      parent = P;
      action = A;
      state = S;
      if (P != NULL)
        opponent = !parent->opponent;
    }

    ~Node() {
      for (Node* child : children) {
        delete child;
      }
    }

    float UCB(float C) const {
      // Upper Confidence Bound
      // Lower C favors exploitation, higher favors exploration
      if (!opponent)
        return wins/visits + C*(sqrt(log(parent->visits)/visits));
      else
        return 1.0 - wins/visits + C*(sqrt(log(parent->visits)/visits));
    }

    //bool operator<(const Node& b) {
    //  return UCB(sqrt(2)) < b.UCB(sqrt(2));
    //}

};

class MonteCarloTree {
  public:
    Node* root;
    Engine E;
    float Ce; // exploration bias parameter
  public:
    MonteCarloTree(bool opp, float c) {
      Ce = c;
      Action A = {0};
      root = new Node(NULL, A, E.getBoardState());
      root->opponent = opp;
    }

    void backpropagate(Node* N, float score) {
      root->visits += 1;
      while (N->parent != NULL) {
        N->wins += score;
        N->visits += 1;
        N = N->parent;
      }
    }

    void expand(Node* N) {
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

    //void setRoot(BoardState BS) {
    //  root->state = BS;
    //}

    float rollout(BoardState BS) {
      vector<Action> A_list;
      while (E.getLegalMoves(&A_list, BS) > 0) {// && BS.winner == -1) {
        int R = rand() % A_list.size();
        BS = E.advance(BS, A_list[R]);
      }
      if (BS.winner == -1)// || BS.winner == 0.5)
        return 0.5;
      return BS.winner;
    }

    void run(float* wins, int* visits, Action* A, bool* stop) {
      while (*stop == false) { // wait for separate thread to end evaluation
        //cout << "stop = false\n";
        Node* N = root;
        //traverse the tree by highest UCB until leaf node
        while (!N->children.empty()) {
          int best_i = 0;
          float best_UCB = N->children[0]->UCB(Ce);
          for (unsigned int i=1; i < N->children.size(); ++i) {
            if (N->children[i]->UCB(Ce) > best_UCB) {
              best_i = i;
              best_UCB = N->children[i]->UCB(Ce);
            }
          }
          N = N->children[best_i];
        }

        //E.printBoard(N->state);

        if (N->visits == 0 || N->endstate) {
          // if node is unvisited or endstate, run rollout and backpropagate
          float win = rollout(N->state);
          backpropagate(N, win);
        }
        else {
          // otherwise, expand leaf node and pick a child node to rollout
          expand(N);
          if (!N->children.empty()) {
            int r = rand() % N->children.size();
            N = N->children[r];
          }
          float win = rollout(N->state);
          backpropagate(N, win);
        }
      }
      //cout << "stop = true\n";
      // return list of best actions
      Node* node = root;
      if (!node->children.empty())
        node = root->children[0];
      for (Node* N : root->children) {
        if (N->UCB(Ce) > node->UCB(Ce))
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
        cout << "error: move not possible\n";
        return false;
      }
      else {
        E.advance(A);
        advance(n);
        return true;
      }
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
