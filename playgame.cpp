#include "montecarlo.h"
#include <thread>
#include <unistd.h>
#include <cstring>
#include <iostream>

int main(int argc, char** argv) {
  bool isWhite = false;
  if (argc < 3) {
    cout << "Usage: playgame agentfile.agent computation_time [black]\n";
    return 0;
  }
  if (argc > 3) {
    if (strcmp(argv[3], "black") == 0) {
      isWhite = true;
    }
  }

  float C;
  char NNfile[256];

  char* agentfile = argv[1];
  ifstream Afile(agentfile);
  if (Afile.is_open()) {
    Afile >> C; // toss training alpha
    Afile >> C; // toss alpha decay ratio
    Afile >> C; // get montecarlo exploration bias
    Afile >> NNfile;
  }
  else {
    cout << "Error: Unable to open file: " << agentfile << endl;
    return 1;
  }

  MonteCarloTree M(isWhite, C, NNfile);
  bool stop = false;
  Action A;
  int visits;
  float wins;
  bool quit = false;

  int n = atoi(argv[2]);
  //thread MCTthread(&MonteCarloTree::run, M, &wins, &visits, &A, &stop);
  //usleep(180*1000000);
  //stop = true;
  //MCTthread.join();

  while (!quit) {
    if (M.root->state.turn %2 != isWhite) {
      // Get Human player's next move
      bool success = false;
      do {
        unsigned char i1, i2, j1, j2;
        cout << "Enter Player's Next Move\n";
        cin.clear();
        cin >> i1 >> j1 >> i2 >> j2;
        //cout << int(i1) << " " << int(j1) << " " << int(i2) << " " << int(j2) << endl;
        i1 -= 'A';
        i2 -= 'A';
        j1 -= '1';
        j2 -= '1';
        //cout << int(i1) << " " << int(j1) << " " << int(i2) << " " << int(j2) << endl;
        Action B = {i1,j1,i2,j2};
        success = M.advance(B);
      } while (!success);

      M.E.printBoard();
      // Check if game has ended
      if (M.root->endstate || M.root->state.winner != -1) {
        M.getMoveList();
        quit = true;
        return 0;
      }
    }

    else {
      // spawn computation thread and run for n seconds
      stop = false;
      thread MCTthread(&MonteCarloTree::Run, &M, &wins, &visits, &A, &stop);
      usleep(n*1000000);
      if (M.E.getBoardState().turn == 1 || M.E.getBoardState().turn == 2)
        usleep(n*1000000);
      stop = true;
      MCTthread.join();
      cout << char('A'+A.i1) << 1+A.j1 << " " << char('A'+A.i2) << 1+A.j2 << endl;
      cout << 100*wins/visits << "\% chance of victory\n";
      cout << "node got " << visits << " of " << M.root->visits << " simulations run\n";
      // Make the MCT's suggested move
      M.advance(A);

      M.E.printBoard();
      // Check if game has ended
      if (M.root->endstate || M.root->state.winner != -1) {
        M.getMoveList();
        quit = true;
        return 0;
      }
    }
  }

  return 0;
}
