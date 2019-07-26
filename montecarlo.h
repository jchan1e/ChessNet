#ifndef MONTECARO_H
#define MONTECARLO_H

#include "Engine/engine.h"
#include "NN/network.h"

#include <queue>
#include <vector>
#include <cmath>

class Node {
  public:
    Node* parent = NULL;
    vector<Node*> children;
    float score = 0;
    int visits = 0;
    Action action;
    BoardState state;

    ~Node() {
      for child : children {
        delete child;
      }
    }
}

class MonteCarlo {
  private:
    nodeTree* root;
    priorityQueue Q;
    Engine E;
  public:
    void backpropagate(Node* N, float score) {
      while (N.parent != NULL) {
        N->score += score;
        N->visits += 1;
        N = N->parent;
      }
    }

    void expand(Node* n) {
      action* arr = NULL;
      int moves = E.getMoves(&arr);
      for (int i=0; i < moves; ++i){
        N = new Node(arr[i]);
        N->parent = n;
        N->state = E.move(n->state, N->action);
        Q.push(N);
      }
      delete arr;
    }

    Action start(gamestate state) {
      root.state = state;

      while (!stop) {
        N = selectBestNode(Queue)
        expand(N)
        C = selectChild(N)
        C.value = rollout(C)
        backpropagate(C)
      }
      return sorted(root.children)[0]
    }

    void advance(Node C) {
      if (C.parent != root)
        advance(C.parent);
      
      for N in root.children {
        if (N != C)
          delete N; // implicitly delete children of N
      }
      delete root;
      root = C;
    }

    float UCB(Node n) {
      float v = n.score;
      if (n.parent == NULL)
        return v;
      return v + C*(math.sqrt(math.ln(n.parent->visits)/n.visits))
    }

}

#endif
