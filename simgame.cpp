#include "montecarlo.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
  float C1 = sqrt(2);
  float C2 = sqrt(2);
  int n = 10;
  string filename = "outfile.game";
  if (argc == 2) {
    n = atoi(argv[1]);
  }
  if (argc == 3) {
    n = atoi(argv[1]);
    filename = argv[2];
  }
  if (argc > 3) {
    C1 = atof(argv[1]);
    C2 = atof(argv[2]);
    n = atoi(argv[3]);
  }
  if (argc == 4) {
    filename = argv[4];
  }

  MonteCarloTree M(true, C1);
  MonteCarloTree N(false, C2);
  bool stop = false;
  Action A;
  int visits;
  float wins;
  bool quit = false;
  //thread MCTthread(&MonteCarloTree::run, M, &wins, &visits, &A, &stop);
  //usleep(2*n*1000000);
  //stop = true;
  //MCTthread.join();

  while (!quit) {
    // spawn computation thread and run for n seconds
    stop = false;
    thread MCTthread1(&MonteCarloTree::Run, &M, &wins, &visits, &A, &stop);
    usleep(n*1000000);
    if (M.E.getBoardState().turn == 1)
      usleep(n*1000000);
    stop = true;
    MCTthread1.join();
    cout << char('A'+A.i1) << 1+A.j1 << " " << char('A'+A.i2) << 1+A.j2 << endl;
    cout << 100*wins/visits << "\% chance of White victory\n";
    cout << "node got " << visits << " of " << M.root->visits << " simulations run\n";
    // Make the MCT's suggested move
    M.advance(A);
    N.advance(A);

    M.E.printBoard();
    // Check if game has ended
    if (M.root->endstate || M.root->state.winner != -1) {
      break;
    }

    // spawn computation thread and run for n seconds
    stop = false;
    thread MCTthread2(&MonteCarloTree::Run, &N, &wins, &visits, &A, &stop);
    usleep(n*1000000);
    if (N.E.getBoardState().turn == 2)
      usleep(n*1000000);
    stop = true;
    MCTthread2.join();
    cout << char('A'+A.i1) << 1+A.j1 << " " << char('A'+A.i2) << 1+A.j2 << endl;
    cout << 100*wins/visits << "\% chance of Black victory\n";
    cout << "node got " << visits << " of " << N.root->visits << " simulations run\n";
    // Make the MCT's suggested move
    M.advance(A);
    N.advance(A);

    N.E.printBoard();
    // Check if game has ended
    if (N.root->endstate || N.root->state.winner != -1) {
      break;
    }

  }

  // Export game log to file
  ofstream outfile(filename);

  string w = "Tie";
  float W = M.root->state.winner;
  if (W == 0.0)
    w = "Black";
  if (W == 1.0)
    w = "White";
  cout << "Winner: " << w << endl;
  outfile << W << endl;

  vector<Action> Hist = M.getMoveList();
  int i = 0;
  for (Action B : Hist) {
    ++i;
    cout << i << "\t" << char('A'+B.i1) << B.j1+1 << " " << char('A'+B.i2) << B.j2+1 << endl;
    outfile << B.i1 << " " << B.j1 << " " << B.i2 << " " << B.j2 << endl;
  }

  outfile.close();

  return 0;
}
