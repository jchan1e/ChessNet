#include "montecarlo.h"
#include <thread>
#include <unistd.h>

int main(int argc, char** argv) {
  float C1 = sqrt(2);
  float C2 = sqrt(2);
  int n = 10;
  if (argc > 2) {
    C1 = atof(argv[1]);
    C2 = atof(argv[2]);
  }
  if (argc > 3) {
    n = atoi(argv[3]);
  }
  if (argc == 2) {
    n = atoi(argv[1]);
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
      vector<Action> Hist = M.getMoveList();
      for (Action B : Hist) {
        cout << char('A'+B.i1) << B.j1+1 << " " << char('A'+B.i2) << B.j2+1 << endl;
      }
      quit = true;
      return 0;
    }

    // spawn computation thread and run for n seconds
    stop = false;
    thread MCTthread2(&MonteCarloTree::Run, &N, &wins, &visits, &A, &stop);
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
      vector<Action> Hist = N.getMoveList();
      for (Action B : Hist) {
        cout << char('A'+B.i1) << B.j1+1 << " " << char('A'+B.i2) << B.j2+1 << endl;
      }
      quit = true;
      return 0;
    }

    //// Get Human player's next move
    //bool success = false;
    //do {
    //  unsigned char i1, i2, j1, j2;
    //  cout << "Enter Black's Next Move\n";
    //  cin.clear();
    //  cin >> i1 >> j1 >> i2 >> j2;
    //  //cout << int(i1) << " " << int(j1) << " " << int(i2) << " " << int(j2) << endl;
    //  i1 -= 'A';
    //  i2 -= 'A';
    //  j1 -= '1';
    //  j2 -= '1';
    //  //cout << int(i1) << " " << int(j1) << " " << int(i2) << " " << int(j2) << endl;
    //  Action B = {i1,j1,i2,j2};
    //  success = M.advance(B);
    //} while (!success);

    //M.E.printBoard();
    //// Check if game has ended
    //if (M.root->endstate || M.root->state.winner != -1) {
    //  M.getMoveList();
    //  quit = true;
    //  return 0;
    //}
  }

  return 0;
}
