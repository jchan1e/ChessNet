#ifndef BOARDSTATE_H
#define BOARDSTATE_H

struct BoardState {
  int board[19][19];
  int captured1;
  int captured2;
  int turn;
};

#endif
