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
  float winner;
  bool w_castle_kingside;
  bool w_castle_queenside;
  bool b_castle_kingside;
  bool b_castle_queenside;
};

inline bool operator==(const BoardState& lhs, const BoardState& rhs) {
  if (lhs.w_castle_kingside != rhs.w_castle_kingside)
    return false;
  if (lhs.w_castle_queenside != rhs.w_castle_queenside)
    return false;
  if (lhs.b_castle_kingside != rhs.b_castle_kingside)
    return false;
  if (lhs.b_castle_queenside != rhs.b_castle_queenside)
    return false;
  for (int i=0; i < 8; ++i) {
    for (int j=0; j < 8; ++j) {
      if (lhs.board[i][j] != rhs.board[i][j])
        return false;
    }
  }
  return true;
}

#endif
