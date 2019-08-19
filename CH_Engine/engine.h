#ifndef ENGINE_H
#define ENGINE_H

#include "boardstate.h"
#include "action.h"
#include <vector>

using namespace std;

class Engine {
  public:

    BoardState state;
    int score;

  //public:
    Engine() {
      score = 0; // komi of 6.5, white wins in a tie
      state.turn = 0;
      state.captured1 = 0;
      state.captured2 = 0;
      for (int i=0; i < 19; ++i) {
        for (int j=0; j < 19; ++j) {
          state.board[i][j] = 0;
        }
      }
    }

    BoardState getBoardState() {
      return state;
    }

    int getLegalMoves(vector<Action>* a_list) {
      int moves = 0;
      int player = 1;
      if (state.turn %2 == 1)
        player = -1;
      for (int i=0; i < 19; ++i) {
        for (int j=0; j < 19; ++j) {
          if (state.board[i][j] == 0) {
            ++moves;
            Action a = {player, i, j};
            a_list->push_back(a);
          }
        }
      }
      return moves;
    }

    int getLegalMoves(vector<Action>* a_list, BoardState BS) {
      int moves = 0;
      int player = 1;
      if (BS.turn %2 == 1)
        player = -1;
      for (int i=0; i < 19; ++i) {
        for (int j=0; j < 19; ++j) {
          if (BS.board[i][j] == 0) {
            ++moves;
            Action a = {player, i, j};
            a_list->push_back(a);
          }
        }
      }
      return moves;
    }

    int check(int board[][19]) {
      return 0;
    }

    void advance(Action a) {
      state.board[a.i][a.j] = a.player;
      state.turn++;
      int captured = check(state.board);
      if (a.player > 0)
        state.captured1 += captured;
      else
        state.captured2 += captured;
    }
};

#endif
