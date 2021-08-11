#include "montecarlo.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
  int n = 10;
  if (argc < 4) {
    cout << "Usage: simgame agentfile1.agent agentfile2.agent gamelogfile.game [simulation_time]\n";
    return 0;
  }
  else if (argc == 5)
    n = atoi(argv[4]);
  char* agentfile1 = argv[1];
  char* agentfile2 = argv[2];
  char* logfile = argv[3];

  float C1, C2, toss;
  char Nfile1[256];
  char Nfile2[256];

  ifstream Afile1(agentfile1);
  if (Afile1.is_open()) {
    Afile1 >> toss; // toss training alpha
    Afile1 >> toss; // toss training alpha decay ratio
    Afile1 >> C1; // get montecarlo bias parameter
    Afile1 >> toss; // toss mutation chance
    Afile1 >> Nfile1;
    Afile1.close();
  }
  else {
    cout << "Error: Unable to open file: " << agentfile1 << endl;
    return 1;
  }

  ifstream Afile2(agentfile2);
  if (Afile2.is_open()) {
    Afile2 >> toss; // toss training alpha
    Afile2 >> toss; // toss training alpha decay ratio
    Afile2 >> C2; // get montecarlo bias parameter
    Afile2 >> toss; // toss mutation chance
    Afile2 >> Nfile2;
    Afile2.close();
  }
  else {
    cout << "Error: Unable to open file: " << agentfile2 << endl;
    return 1;
  }

  MonteCarloTree M(true, C1, Nfile1);
  MonteCarloTree N(false, C2, Nfile2);
  bool stop = false;
  Action A_N;
  Action A_M;
  int visits_M;
  int visits_N;
  float wins_M;
  float wins_N;
  bool quit = false;

  //cout << "Beginning Simulation\n";
  while (!quit) {
    // spawn computation thread and run for n seconds
    stop = false;
    thread MCTthread1(&MonteCarloTree::Run, &M, &wins_M, &visits_M, &A_M, &stop);
    thread MCTthread2(&MonteCarloTree::Run, &N, &wins_N, &visits_N, &A_N, &stop);
    int iter = 0;
    while (!stop && iter < n) {
      usleep(1000000);
      M.root->mtx.lock();
      //cout << "Total:\t" << M.root->visits << endl;
      if (M.root->visits >= 10000){
        //cout << "Nodes:\t";
        for (Node* n : M.root->children) {
          //cout << n->visits << "\t";
          if ((float)n->visits/M.root->visits > 0.5) {
            stop = true;
          }
        }
        //cout << endl;
      }
      M.root->mtx.unlock();
      iter++;
    }
    //usleep(n*1000000);
    //if (M.E.getBoardState().turn == 1)
    //  usleep(n*1000000);
    stop = true;
    MCTthread1.join();
    MCTthread2.join();
    //cout << char('A'+A_M.i1) << 1+A_M.j1 << " " << char('A'+A_M.i2) << 1+A_M.j2 << endl;
    //cout << 100*wins_M/visits_M << "\% chance of White victory\n";
    //cout << "node got " << visits_M << " of " << M.root->visits << " simulations run\n";
    // Make the MCT's suggested move
    M.advance(A_M);
    N.advance(A_M);

    M.E.printBoard();
    // Check if game has ended
    if (M.root->endstate || M.root->state.winner != -1) {
      break;
    }

    // spawn computation thread and run for n seconds
    stop = false;
    thread MCTthread3(&MonteCarloTree::Run, &N, &wins_M, &visits_M, &A_M, &stop);
    thread MCTthread4(&MonteCarloTree::Run, &N, &wins_N, &visits_N, &A_N, &stop);
    iter = 0;
    while (!stop && iter < n) {
      usleep(1000000);
      N.root->mtx.lock();
      //cout << "Total:\t" << N.root->visits << endl;
      if (N.root->visits >= 10000){
        //cout << "Nodes:\t";
        for (Node* n : N.root->children) {
          //cout << n->visits << "\t";
          if ((float)n->visits/N.root->visits > 0.5) {
            stop = true;
          }
        }
        //cout << endl;
      }
      N.root->mtx.unlock();
      iter++;
    }
    //usleep(n*1000000);
    //if (N.E.getBoardState().turn == 2)
    //  usleep(n*1000000);
    stop = true;
    MCTthread3.join();
    MCTthread4.join();
    //cout << char('A'+A_N.i1) << 1+A_N.j1 << " " << char('A'+A_N.i2) << 1+A_N.j2 << endl;
    //cout << 100*wins_N/visits_N << "\% chance of Black victory\n";
    //cout << "node got " << visits_N << " of " << N.root->visits << " simulations run\n";
    // Make the MCT's suggested move
    M.advance(A_N);
    N.advance(A_N);

    N.E.printBoard();
    // Check if game has ended
    if (N.root->endstate || N.root->state.winner != -1) {
      break;
    }

  }

  // Export game log to file
  ofstream outfile(logfile);

  string w = "Tie";
  float W = M.root->state.winner;
  if (W == -1.0)
    W = 0.5;
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
    //cout << i << "\t" << char('A'+B.i1) << B.j1+1 << " " << char('A'+B.i2) << B.j2+1 << endl;
    outfile << B.i1 << " " << B.j1 << " " << B.i2 << " " << B.j2 << endl;
  }

  outfile.close();

  return 0;
}
