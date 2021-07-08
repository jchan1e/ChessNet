
#include <iostream>
#include <fstream>
#include "Engine/engine.h"
#include "NN/neuralnet.h"

int main (int argc, char** argv) {
  if (argc < 4) {
    cout << "Usage: train agentfile num_epochs gamelog [gamelog ...]\n";
    return 0;
  }

  Engine E;
  vector<float*> gamestates;
  vector<float*> outcomes;
  char* agentfile = argv[1];
  float alpha;
  float decay_ratio;
  float C;
  float mutate;
  char NNfile[256];
  ifstream Afile(agentfile);
  if (Afile.is_open()) {
    Afile >> alpha;
    Afile >> decay_ratio;
    Afile >> C;
    Afile >> mutate;
    Afile >> NNfile;
  }

  int num_epochs = atoi(argv[2]);
  for (int i=5; i < argc; ++i) {
    char* datafile = argv[i];
    ifstream data(datafile);

    if (!data.is_open()) {
      cerr << "could not open gamelog " << i-5 << endl;
      return 1;
    }
    else {
      float winner;
      data >> winner;
      while (!data.eof()) {
        int a, b, c, d;
        data >> a >> b >> c >> d;
        Action A = {a,b,c,d};
        E.advance(A);
        BoardState BS = E.getBoardState();
        float* BS_array = new float[12*64 + 1 + 4];
        for (int x=0; x < 8; ++x) {
          for (int y=0; y < 8; ++y) {
            switch (BS.board[x][y]) {
              int l;
              case w_pawn:
                l = 0;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case w_bishop:
                l = 1;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case w_knight:
                l = 2;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case w_rook:
                l = 3;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case w_queen:
                l = 4;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case w_king:
                l = 5;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case b_pawn:
                l = 6;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case b_bishop:
                l = 7;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case b_knight:
                l = 8;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case b_rook:
                l = 9;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case b_queen:
                l = 10;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              case b_king:
                l = 11;
                BS_array[64*l + 8*x + y] = 1.0;
                break;
              default: // blank
                break;
            }
          }
        }
        BS_array[12*64 + 0] = -1.0 + 2.0*(BS.turn%2);
        BS_array[12*64 + 1] = BS.w_castle_kingside?1.0:0.0;
        BS_array[12*64 + 2] = BS.w_castle_queenside?1.0:0.0;
        BS_array[12*64 + 3] = BS.b_castle_kingside?1.0:0.0;
        BS_array[12*64 + 4] = BS.b_castle_queenside?1.0:0.0;

        float* W = new float[2];
        W[0] = winner;
        W[1] = 1.0-winner;

        gamestates.push_back(BS_array);
        outcomes.push_back(W);
      }
    }
  }

  // instantiate neuralnet from agentfile
  Neuralnet N(NNfile);
  // train agent on data vectors
  N.train(gamestates, outcomes, num_epochs, alpha, alpha*decay_ratio);
  // export trained agent to file
  N.save(NNfile);

  // clean up
  for (float* BS_array : gamestates)
    delete BS_array;
  for (float* W : outcomes)
    delete W;

  return 0;
}
