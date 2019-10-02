#ifndef ENGINE_H
#define ENGINE_H

#include "boardstate.h"
#include "action.h"
#include <vector>
#include <iostream>

using namespace std;

class Engine {
  private:

    BoardState state;
    vector<Action> hist;

    void check_pawns(BoardState* BS) {
      for (int i=0; i < 8; ++i) {
        if (BS->board[i][0] == b_pawn)
          BS->board[i][0] = b_queen;
        if (BS->board[i][7] == w_pawn)
          BS->board[i][7] = w_queen;
      }
    }

    float check_kings(BoardState BS) {
      int w = 0;
      for (int i=0; i < 8; ++i) {
        for (int j=0; j < 8; ++j) {
          if (BS.board[i][j] == w_king)
            w++;
          else if (BS.board[i][j] == b_king)
            w--;
        }
      }
      if (w)
        return 0.5 + 0.5*w;
      else
        return -1.0;
    }

    bool check_stalemate(BoardState BS) {
      if (BS.turn > 400)
        return true;

      int Pcount = 0;
      int Rcount = 0;
      int Bwcount = 0;
      int Bbcount = 0;
      int Ncount = 0;
      int Qcount = 0;
      int Kb = 0;
      int Kw = 0;

      for (int i=0; i < 8; ++i) {
        for (int j=0; j < 8; ++j) {
          switch (BS.board[i][j]) {
            case b_pawn:
            case w_pawn:
              Pcount++;
              break;
            case b_rook:
            case w_rook:
              Rcount++;
              break;
            case b_bishop:
            case w_bishop:
              if ((i+j)%2 == 0)
                Bbcount++; // bishop on black squares
              else
                Bwcount++; // bishop on white squares
              break;
            case b_knight:
            case w_knight:
              Ncount++;
              break;
            case b_queen:
            case w_queen:
              Qcount++;
              break;
            case b_king:
              Kb++;
              break;
            case w_king:
              Kw++;
              break;
            default:
              break;
          }
        }
      }
      if (Kb < 1 || Kw < 1)
        return false;
      else if (Pcount == 0 && Rcount == 0 && Qcount == 0) {
        // Unwinnable game states
        if (Bbcount+Bwcount == 0 && Ncount == 0) // only 2 kings left
          return true;
        else if (Bbcount+Bwcount == 1 && Ncount == 0) // 2 kings + bishop
          return true;
        else if (Bbcount+Bwcount == 0 && Ncount == 1) // 2 kings + knight
          return true;
        else if (Bbcount+Bwcount == 2 && Ncount == 0 && Bbcount != Bwcount) // 2 bishops on the same tiles
          return true;
      }
      return false;
    }

  public:
    Engine() {
      state.turn = 1;
      state.winner = -1;
      for (int i=0; i < 8; ++i) {
        for (int j=0; j < 8; ++j) {
          state.board[i][j] = blank;
          if (j == 1)
            state.board[i][j] = w_pawn;
          else if (j == 6)
            state.board[i][j] = b_pawn;
          else if (j == 0) {
            if (i == 0 || i == 7)
              state.board[i][j] = w_rook;
            else if (i == 1 || i == 6)
              state.board[i][j] = w_knight;
            else if (i == 2 || i == 5)
              state.board[i][j] = w_bishop;
            else if (i == 3)
              state.board[i][j] = w_queen;
            else // i == 4
              state.board[i][j] = w_king;
          }
          else if (j == 7) {
            if (i == 0 || i == 7)
              state.board[i][j] = b_rook;
            else if (i == 1 || i == 6)
              state.board[i][j] = b_knight;
            else if (i == 2 || i == 5)
              state.board[i][j] = b_bishop;
            else if (i == 3)
              state.board[i][j] = b_queen;
            else // i == 4
              state.board[i][j] = b_king;
          }
        }
      }
    }

    BoardState getBoardState() {
      return state;
    }

    int getLegalMoves(vector<Action>* a_list) {
      return getLegalMoves(a_list, state);
    }

    int getLegalMoves(vector<Action>* a_list, BoardState BS) {
      int moves = 0;
      a_list->clear();
      for (int i=0; i < 8; ++i) {
        for (int j=0; j < 8; ++j) {
          if (BS.board[i][j] != blank) {
            if (BS.turn%2 == 0) { // black's turn
              int ii, jj;
              switch(BS.board[i][j]) {
                case b_pawn:
                  if (j-1 >= 0 && BS.board[i][j-1] == blank) { // straight ahead
                    Action a = {i, j, i, j-1};
                    a_list->push_back(a);
                    moves++;
                  }
                  if (j == 6 && BS.board[i][j-1] == blank && BS.board[i][j-2] == blank) { // double first move
                    Action a = {i, j, i, j-2};
                    a_list->push_back(a);
                    moves++;
                  }
                  if (j-1 >= 0 && i-1 >= 0 && BS.board[i-1][j-1] > blank) { // blank = 0
                    Action a = {i, j, i-1, j-1};                           // white pieces > 0
                    a_list->push_back(a);
                    moves++;
                  }
                  if (j-1 >= 0 && i+1 < 8 && BS.board[i+1][j-1] > blank) {
                    Action a = {i, j, i+1, j-1};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;

                case b_knight:
                  // check each of 8 possible moves individually
                  ii = i+1; jj = j+2;
                  if (ii < 8 && jj < 8 &&
                      BS.board[ii][jj] >= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+2; jj = j+1;
                  if (ii < 8 && jj < 8 &&
                      BS.board[ii][jj] >= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+2; jj = j+-1;
                  if (ii < 8 && jj >= 0 &&
                      BS.board[ii][jj] >= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+1; jj = j+-2;
                  if (ii < 8 && jj >= 0 &&
                      BS.board[ii][jj] >= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+-1; jj = j+-2;
                  if (ii >= 0 && jj >= 0 &&
                      BS.board[ii][jj] >= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+-2; jj = j+-1;
                  if (ii >= 0 && jj >= 0 &&
                      BS.board[ii][jj] >= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+-2; jj = j+1;
                  if (ii >= 0 && jj < 8 &&
                      BS.board[ii][jj] >= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+-1; jj = j+2;
                  if (ii >= 0 && jj < 8 &&
                      BS.board[ii][jj] >= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;

                case b_bishop:
                  // upper right
                  ii = i+1;
                  jj = j+1;
                  while (ii < 8 && jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                    jj++;
                  }
                  if (ii < 8 && jj < 8 &&
                      BS.board[ii][jj] > blank) { // stopped because of enemy piece
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // lower right
                  ii = i+1;
                  jj = j-1;
                  while (ii < 8 && 0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                    jj--;
                  }
                  if (ii < 8 && 0 <= jj &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // lower left
                  ii = i-1;
                  jj = j-1;
                  while (0 <= ii && 0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                    jj--;
                  }
                  if (0 <= ii && 0 <= jj &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // upper left
                  ii = i-1;
                  jj = j+1;
                  while (0 <= ii && jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                    jj++;
                  }
                  if (0 <= ii && jj < 8 &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;

                case b_rook:
                  // up
                  ii = i;
                  jj = j+1;
                  while (jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    jj++;
                  }
                  if (jj < 8 &&
                      BS.board[ii][jj] > blank) { // stopped because of enemy piece
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // right
                  ii = i+1;
                  jj = j;
                  while (ii < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                  }
                  if (ii < 8 &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // down
                  ii = i;
                  jj = j-1;
                  while (0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    jj--;
                  }
                  if (0 <= jj &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // left
                  ii = i-1;
                  jj = j;
                  while (0 <= ii &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                  }
                  if (0 <= ii &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;
                case b_queen:
                  // bishop + rook
                  // upper right
                  ii = i+1;
                  jj = j+1;
                  while (ii < 8 && jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                    jj++;
                  }
                  if (ii < 8 && jj < 8 &&
                      BS.board[ii][jj] > blank) { // stopped because of enemy piece
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // lower right
                  ii = i+1;
                  jj = j-1;
                  while (ii < 8 && 0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                    jj--;
                  }
                  if (ii < 8 && 0 <= jj &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // lower left
                  ii = i-1;
                  jj = j-1;
                  while (0 <= ii && 0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                    jj--;
                  }
                  if (0 <= ii && 0 <= jj &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // upper left
                  ii = i-1;
                  jj = j+1;
                  while (0 <= ii && jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                    jj++;
                  }
                  if (0 <= ii && jj < 8 &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // up
                  ii = i;
                  jj = j+1;
                  while (jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    jj++;
                  }
                  if (jj < 8 &&
                      BS.board[ii][jj] > blank) { // stopped because of enemy piece
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // right
                  ii = i+1;
                  jj = j;
                  while (ii < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                  }
                  if (ii < 8 &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // down
                  ii = i;
                  jj = j-1;
                  while (0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    jj--;
                  }
                  if (0 <= jj &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // left
                  ii = i-1;
                  jj = j;
                  while (0 <= ii &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                  }
                  if (0 <= ii &&
                      BS.board[ii][jj] > blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;

                case b_king:
                  for (int ii=i-1; ii<= i+1; ++ii) {
                    for (int jj=j-1; jj <=j+1; ++jj) {
                      if (0 <= ii && ii < 8 &&
                          0 < jj && jj < 8 &&
                          BS.board[ii][jj] >= blank) {
                        Action a = {i, j, ii, jj};
                        a_list->push_back(a);
                        moves++;
                      }
                    }
                  }
                  break;

                default: // white pieces on black's turn
                  break;
              }
            }
            else { // white
              int ii, jj;
              switch (BS.board[i][j]) {
                case w_pawn:
                  if (j+1 < 8 && BS.board[i][j+1] == blank) { // straight ahead
                    Action a = {i, j, i, j+1};
                    a_list->push_back(a);
                    moves++;
                  }
                  if (j == 1 && BS.board[i][j+1] == blank && BS.board[i][j+2] == blank) { // double first move
                    Action a = {i, j, i, j+2};
                    a_list->push_back(a);
                    moves++;
                  }
                  if (j+1 < 8 && i-1 >= 0 && BS.board[i-1][j+1] < blank) { // blank = 0
                    Action a = {i, j, i-1, j+1};                           // black pieces < 0
                    a_list->push_back(a);
                    moves++;
                  }
                  if (j+1 < 8 && i+1 < 8 && BS.board[i+1][j+1] < blank) {
                    Action a = {i, j, i+1, j+1};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;

                case w_knight:
                  // check each of 8 possible moves individually
                  ii = i+1; jj = j+2;
                  if (ii < 8 && jj < 8 &&
                      BS.board[ii][jj] <= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+2; jj = j+1;
                  if (ii < 8 && jj < 8 &&
                      BS.board[ii][jj] <= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+2; jj = j-1;
                  if (ii < 8 && jj >= 0 &&
                      BS.board[ii][jj] <= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i+1; jj = j-2;
                  if (ii < 8 && jj >= 0 &&
                      BS.board[ii][jj] <= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i-1; jj = j-2;
                  if (ii >= 0 && jj >= 0 &&
                      BS.board[ii][jj] <= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i-2; jj = j-1;
                  if (ii >= 0 && jj >= 0 &&
                      BS.board[ii][jj] <= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i-2; jj = j+1;
                  if (ii >= 0 && jj < 8 &&
                      BS.board[ii][jj] <= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  ii = i-1; jj = j+2;
                  if (ii >= 0 && jj < 8 &&
                      BS.board[ii][jj] <= blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;

                case w_bishop:
                  // upper right
                  ii = i+1;
                  jj = j+1;
                  while (ii < 8 && jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                    jj++;
                  }
                  if (ii < 8 && jj < 8 &&
                      BS.board[ii][jj] < blank) { // stopped because of enemy piece
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // lower right
                  ii = i+1;
                  jj = j-1;
                  while (ii < 8 && 0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                    jj--;
                  }
                  if (ii < 8 && 0 <= jj &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // lower left
                  ii = i-1;
                  jj = j-1;
                  while (0 <= ii && 0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                    jj--;
                  }
                  if (0 <= ii && 0 <= jj &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // upper left
                  ii = i-1;
                  jj = j+1;
                  while (0 <= ii && jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                    jj++;
                  }
                  if (0 <= ii && jj < 8 &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;

                case w_rook:
                  // up
                  ii = i;
                  jj = j+1;
                  while (jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    jj++;
                  }
                  if (jj < 8 &&
                      BS.board[ii][jj] < blank) { // stopped because of enemy piece
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // right
                  ii = i+1;
                  jj = j;
                  while (ii < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                  }
                  if (ii < 8 &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // down
                  ii = i;
                  jj = j-1;
                  while (0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    jj--;
                  }
                  if (0 <= jj &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // left
                  ii = i-1;
                  jj = j;
                  while (0 <= ii &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                  }
                  if (0 <= ii &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;
                case w_queen:
                  // bishop + rook
                  // upper right
                  ii = i+1;
                  jj = j+1;
                  while (ii < 8 && jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                    jj++;
                  }
                  if (ii < 8 && jj < 8 &&
                      BS.board[ii][jj] < blank) { // stopped because of enemy piece
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // lower right
                  ii = i+1;
                  jj = j-1;
                  while (ii < 8 && 0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                    jj--;
                  }
                  if (ii < 8 && 0 <= jj &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // lower left
                  ii = i-1;
                  jj = j-1;
                  while (0 <= ii && 0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                    jj--;
                  }
                  if (0 <= ii && 0 <= jj &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // upper left
                  ii = i-1;
                  jj = j+1;
                  while (0 <= ii && jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                    jj++;
                  }
                  if (0 <= ii && jj < 8 &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // up
                  ii = i;
                  jj = j+1;
                  while (jj < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    jj++;
                  }
                  if (jj < 8 &&
                      BS.board[ii][jj] < blank) { // stopped because of enemy piece
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // right
                  ii = i+1;
                  jj = j;
                  while (ii < 8 &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii++;
                  }
                  if (ii < 8 &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // down
                  ii = i;
                  jj = j-1;
                  while (0 <= jj &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    jj--;
                  }
                  if (0 <= jj &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  // left
                  ii = i-1;
                  jj = j;
                  while (0 <= ii &&
                         BS.board[ii][jj] == blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                    ii--;
                  }
                  if (0 <= ii &&
                      BS.board[ii][jj] < blank) {
                    Action a = {i, j, ii, jj};
                    a_list->push_back(a);
                    moves++;
                  }
                  break;

                case w_king:
                  for (int ii=i-1; ii<= i+1; ++ii) {
                    for (int jj=j-1; jj <=j+1; ++jj) {
                      if (0 <= ii && ii < 8 &&
                          0 < jj && jj < 8 &&
                          BS.board[ii][jj] <= blank) {
                        Action a = {i, j, ii, jj};
                        a_list->push_back(a);
                        moves++;
                      }
                    }
                  }
                  break;

                default: //shouldn't be possible to reach this line
                  break;
              }
            }
          }
        }
      }

      float winner = -1.0;

      if (moves == 0) {
        if (player_stalemate(BS))
          winner = 0.5;
      }

      float w = check_kings(BS);
      if (w != -1) {
        winner = w;
      }

      if (winner != -1.0) {
        a_list->clear();
        moves = -1;
      }
      return moves;
    }

    bool player_stalemate(BoardState BS) {
      // if active player has no moves and is not in check, stalemate
      bool stalemate = true;
      int player = BS.turn%2 - 1;
      int ki = 0;
      int kj = 0;
      // sanity check that both kings are present
      int kings = 0;
      for (int i=0; i < 8; ++i) {
        for (int j=0; j < 8; ++j) {
          if (BS.board[i][j] == player*w_king) {
            kings++;
          }
        }
      }
      if (kings != 2)
        return false;
      for (int i=0; i < 8; ++i) {
        for (int j=0; j < 8; ++j) {
          if (BS.board[i][j] == player*w_king) {
            ki = i;
            kj = j;
          }
        }
      }
      // upper right
      int ii = ki+1;
      int jj = kj+1;
      while (ii < 8 && jj < 8 &&
             BS.board[ii][jj] == blank) {
        ii++;
        jj++;
      }
      if (ii < 8 && jj < 8 &&
          BS.board[ii][jj] != blank) {
        if (BS.board[ii][jj] == player*b_queen ||
            BS.board[ii][jj] == player*b_bishop)
          stalemate = false;
      }
      // lower right
      ii = ki+1;
      jj = kj-1;
      while (ii < 8 && 0 <= jj &&
             BS.board[ii][jj] == blank) {
        ii++;
        jj--;
      }
      if (ii < 8 && 0 <= jj &&
          BS.board[ii][jj] != blank) {
        if (BS.board[ii][jj] == player*b_queen ||
            BS.board[ii][jj] == player*b_bishop)
          stalemate = false;
      }
      // lower left
      ii = ki-1;
      jj = kj-1;
      while (0 <= ii && 0 <= jj &&
             BS.board[ii][jj] == blank) {
        ii--;
        jj--;
      }
      if (0 <= ii && 0 <= jj &&
          BS.board[ii][jj] != blank) {
        if (BS.board[ii][jj] == player*b_queen ||
            BS.board[ii][jj] == player*b_bishop)
          stalemate = false;
      }
      // upper left
      ii = ki-1;
      jj = kj+1;
      while (0 <= ii && jj < 8 &&
             BS.board[ii][jj] == blank) {
        ii--;
        jj++;
      }
      if (0 <= ii && jj < 8 &&
          BS.board[ii][jj] != blank) {
        if (BS.board[ii][jj] == player*b_queen ||
            BS.board[ii][jj] == player*b_bishop)
          stalemate = false;
      }
      // up
      ii = ki;
      jj = kj+1;
      while (jj < 8 &&
             BS.board[ii][jj] == blank) {
        jj++;
      }
      if (jj < 8 &&
          BS.board[ii][jj] != blank) {
        if (BS.board[ii][jj] == player*b_queen ||
            BS.board[ii][jj] == player*b_rook)
          stalemate = false;
      }
      // right
      ii = ki+1;
      jj = kj;
      while (ii < 8 &&
             BS.board[ii][jj] == blank) {
        ii++;
      }
      if (ii < 8 &&
          BS.board[ii][jj] != blank) {
        if (BS.board[ii][jj] == player*b_queen ||
            BS.board[ii][jj] == player*b_rook)
          stalemate = false;
      }
      // down
      ii = ki;
      jj = kj-1;
      while (0 <= jj &&
             BS.board[ii][jj] == blank) {
        jj--;
      }
      if (0 <= jj &&
          BS.board[ii][jj] != blank) {
        if (BS.board[ii][jj] == player*b_queen ||
            BS.board[ii][jj] == player*b_rook)
          stalemate = false;
      }
      // left
      ii = ki-1;
      jj = kj;
      while (0 <= ii &&
             BS.board[ii][jj] == blank) {
        ii--;
      }
      if (0 <= ii &&
          BS.board[ii][jj] != blank) {
        if (BS.board[ii][jj] == player*b_queen ||
            BS.board[ii][jj] == player*b_rook)
          stalemate = false;
      }
      // check each of 8 possible knight moves individually
      ii = ki+1; jj = kj+2;
      if (ii < 8 && jj < 8 &&
          BS.board[ii][jj] == player*b_knight) {
        stalemate = false;
      }
      ii = ki+2; jj = kj+1;
      if (ii < 8 && jj < 8 &&
          BS.board[ii][jj] == player*b_knight) {
        stalemate = false;
      }
      ii = ki+2; jj = kj-1;
      if (ii < 8 && jj >= 0 &&
          BS.board[ii][jj] == player*b_knight) {
        stalemate = false;
      }
      ii = ki+1; jj = kj-2;
      if (ii < 8 && jj >= 0 &&
          BS.board[ii][jj] == player*b_knight) {
        stalemate = false;
      }
      ii = ki-1; jj = kj-2;
      if (ii >= 0 && jj >= 0 &&
          BS.board[ii][jj] == player*b_knight) {
        stalemate = false;
      }
      ii = ki-2; jj = kj-1;
      if (ii >= 0 && jj >= 0 &&
          BS.board[ii][jj] == player*b_knight) {
        stalemate = false;
      }
      ii = ki-2; jj = kj+1;
      if (ii >= 0 && jj < 8 &&
          BS.board[ii][jj] == player*b_knight) {
        stalemate = false;
      }
      ii = ki-1; jj = kj+2;
      if (ii >= 0 && jj < 8 &&
          BS.board[ii][jj] == player*b_knight) {
        stalemate = false;
      }

      return stalemate;
    }

    void advance(Action a) {
      hist.push_back(a);
      state = advance(state, a);
    }

    BoardState advance(BoardState BS, Action a) {
      // new position = old position
      // old position = blank
      BoardState S = BS;
      S.board[a.i2][a.j2] = S.board[a.i1][a.j1];
      S.board[a.i1][a.j1] = blank;
      check_pawns(&S);
      if (check_stalemate(S))
        S.winner = 0.5;
      float w = check_kings(S);
      if (w != -1.0)
        S.winner = w;
      S.turn++;
      return S;
    }

    vector<Action> getHist() {
      return hist;
    }

    void printBoard() {
      printBoard(state);
    }

    void printBoard(BoardState BS) {
      cout << "  ┌───┬───┬───┬───┬───┬───┬───┬───┐\n";
      for (int j = 7; j >= 0; --j) {
        cout << char('1'+j) << " │";
        for (int i = 0; i < 8; ++i) {
          switch (BS.board[i][j]) {
            case w_pawn:
              cout << " P │";
              break;
            case w_bishop:
              cout << " B │";
              break;
            case w_knight:
              cout << " N │";
              break;
            case w_rook:
              cout << " R │";
              break;
            case w_queen:
              cout << " Q │";
              break;
            case w_king:
              cout << " K │";
              break;
            case b_pawn:
              cout << " p │";
              break;
            case b_bishop:
              cout << " b │";
              break;
            case b_knight:
              cout << " n │";
              break;
            case b_rook:
              cout << " r │";
              break;
            case b_queen:
              cout << " q │";
              break;
            case b_king:
              cout << " k │";
              break;
            default: // blank
              cout << "   │";
              break;
          }
        }
        cout << "\n";
        if (j != 0)
          cout << "  ├───┼───┼───┼───┼───┼───┼───┼───┤\n";
      }
      cout << "  └───┴───┴───┴───┴───┴───┴───┴───┘\n";
      cout << "    A   B   C   D   E   F   G   H  \n";
    }

};

#endif
