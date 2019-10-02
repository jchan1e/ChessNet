#ifndef BOARDSTATE_H
#define BOARDSTATE_H

enum piece : char {
  w_pawn = 1,
  w_bishop = 2,
  w_knight = 3,
  w_rook = 4,
  w_queen = 5,
  w_king = 6,
  b_pawn = -1,
  b_bishop = -2,
  b_knight = -3,
  b_rook = -4,
  b_queen = -5,
  b_king = -6,
  blank = 0
};

struct BoardState {
  piece board[8][8];
  short int turn;
  char winner;
};

#endif
