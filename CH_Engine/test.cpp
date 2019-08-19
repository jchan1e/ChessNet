#include <cstdlib>
#include <iostream>
#include "engine.h"

using namespace std;

int main() {
  //FUNDAMENTALS
  //should instantiate
  Engine* E = new Engine();
  if (E != NULL)
    cout << "+ Instantiates\n";
  else
    cout << "- Failed to instantiate\n";

  //should return board state
  BoardState S = E->state;
  bool success = true;
  for (int i=0; i < 19; ++i) {
    for (int j=0; j < 19; ++j) {
      if (S.board[i][j] != 0)
        success = false;
    }
  }
  if (S.captured1 == 0 && S.captured2 == 0 && S.turn == 0 && success)
    cout << "+ returned valid boardstate\n";
  else
    cout << "- returned invalid boardstate\n";

  //should retun list of legal moves
  vector<Action> movelist;
  int num_moves = E->getLegalMoves(&movelist);
  if (movelist.size() == num_moves == 19*19)
    cout << "+ returned valid movelist\n";
  else
    cout << "- returned invalid movelist\n";

  //should it detect game end states?

  //BASIC RULES

}
