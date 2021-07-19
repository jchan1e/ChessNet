#include <iostream>
#include <fstream>
#include "NN/neuralnet.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "Usage: createAgent filename [hiddenlayer1_size [...]]\n";
    return 0;
  }

  int num_layers = argc;
  shape S;
  S.n = num_layers;
  S.sizes[0] = 12*64+5;
  S.sizes[num_layers-1] = 2;
  for (int i=1; i < num_layers-1; ++i) {
    S.sizes[i] = atoi(argv[i+1]);
  }
  S.sigm = true;
  S.lam = 0.000001;

  Neuralnet N(&S);
  N.save(argv[1]);

  //delete sizes;

  return 0;
}
