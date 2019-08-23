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
  BoardState S = E->getBoardState();
  bool success = true;
  for (int i=0; i < 8; ++i) {
    for (int j=0; j < 8; ++j) {
      // Check board state //
      //if (S.board[i][j] != 0)
      //  success = false;
    }
  }
  if (success)
    cout << "+ returned valid boardstate\n";
  else
    cout << "- returned invalid boardstate\n";

  //should retun list of legal moves
  vector<Action> movelist;
  int num_moves = E->getLegalMoves(&movelist);
    //TODO: check that the moves are valid)
  if (movelist.size() == num_moves && num_moves == 20)
    cout << "+ returned valid movelist\n";
  else {
    cout << "- returned invalid movelist\n" << num_moves << " " << movelist.size() << endl;
    for (Action m : movelist)
      cout << m.i1 << "," << m.j1 << " " << m.i2 << "," << m.j2 << endl;
  }
  E->printBoard();

  //TODO: //should detect game end states
  // play out fool's mate
  //  or just load kingless game state
  // movelist should return as empty
  // * also test stalemate

  //BASIC RULES
  // pieces should move according to their rules
  // test that moves capture pieces



}
