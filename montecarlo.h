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
    }

    ~Node() {
      for (Node* child : children) {
        delete child;
      }
    }

    float UCB(float C) const {
      // Upper Confidence Bound
      // C balances exploring new nodes vs exploiting good ones
      return wins/visits + C*(sqrt(log(parent->visits)/visits));
    }

    bool operator<(const Node& b) {
      return UCB(sqrt(2)) < b.UCB(sqrt(2));
    }

};

class MonteCarloTree {
  private:
    Node* root;
    Engine E;
    float Ce; // exploration bias parameter
  public:
    MonteCarloTree(float c) {
      Ce = c;
      Action A = {0,0,0,0};
      root = new Node(NULL, A, E.getBoardState());
    }

    void backpropagate(Node* N, float score) {
      while (N->parent != NULL) {
        N->wins += score;
        N->visits += 1;
        N = N->parent;
      }
    }

    void expand(Node* n) {
      vector<Action> A_list;
      if (E.getLegalMoves(&A_list) > 0) {
        int moves = A_list.size();
        for (int i=0; i < moves; ++i){
          Node* N = new Node(n, A_list[i], E.advance(n->state, A_list[i]));
          n->children.push_back(N);
        }
      else {
        n->endstate = true;
        if (E.player_stalemate(&(n->state))) {
          n->
    }

    //void setRoot(BoardState BS) {
    //  root->state = BS;
    //}

    int rollout(BoardState BS) {
      vector<Action> A_list;
      while (-1 != E.getLegalMoves(&A_list, BS)) {
        int R = rand() % A_list.size();
        BS = E.advance(BS, A_list[R]);
      }
      return BS.winner;
    }

    Action run(bool* stop) {
      while (*stop == false) { // wait for separate thread to end evaluation
        Node* N = root;
        while (!N->children.empty()) {
          //traverse the tree by highest UCB til leaf node
          int best_i = 0;
          float best_UCB = N->children[0]->UCB(Ce);
          for (int i=1; i < N->children.size(); ++i)
          {
            if (N->children[i]->UCB(Ce) > best_UCB) {
              best_i = i;
              best_UCB = N->children[i]->UCB(Ce);
            }
          }
          N = N->children[best_i];
        }

        if (N->visits == 0) {
          // run rollout and backpropagate
          BoardState BS = N->state;
          int w = rollout(BS);
        }
        else {
          // expand leaf node and pick a child node to rollout
          expand(N);
          int r = rand() % N->children.size();
          Node* C = N->children[r];
          //float win = rollout(C);
          float win = 0.5;
          backpropagate(C, win);
        }
      }
      return root->children[0]->action;
    }

    void advance(Node* R) {
      if (R->parent != root)
        advance(R->parent);

      for (Node* N : root->children) {
        if (N != R)
          delete N; // implicitly delete children of N
      }
      root->children.clear();
      delete root;
      root = R;
    }

    //float UCB(Node n) {
    //  float v = n->score;
    //  if (n->parent == NULL)
    //    return v;
    //  return v + C*(math.sqrt(math.ln(n.parent->visits)/n.visits))
    //}

};

#endif
