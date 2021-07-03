// Only creates and initializes the neuralnetwork file.
// Use other scripts to create the .agent file

#include <iostream>
#include <fstream>
#include "NN/neuralnet.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "Usage: createAgent filename [hiddenlayer1_size [...]]\n";
    return 0;
  }

  int num_layers = argc;
  int* sizes = new int[num_layers];
  sizes[0] = 12*64+5;
  sizes[num_layers-1] = 2;
  for (int i=1; i < num_layers-1; ++i) {
    sizes[i] = atoi(argv[i+1]);
  }

  shape S = {num_layers, sizes};
  Neuralnet N(&S);
  N.save(argv[1]);

  delete sizes;

  return 0;
}