#include "AbHS.h"
#include "Util.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <unordered_set>

using namespace std;

AbHS * solver;

void stop(int) {
  printf("s UNKNOWN\n");
  fflush(stdout);
  _Exit(1);
}

int main(int argc, const char* argv[]) {

  clock_t begin = clock();

  condTerminate(argc == 1, 1, "Error: need filepath.\n");

  signal(SIGINT, stop);
  signal(SIGTERM, stop);
  signal(SIGXCPU, stop);

  ifstream file(argv[1]);
  AbdInstance instance(file);
  solver = new AbHS(instance);

  file.close();

  unordered_set<string> flags; 
  vector<string> args(argc);
  args.assign(argv+2, argv+argc);
  for (auto s : args) flags.insert(s);
  
  bool exclude = flags.count("-e");
  bool verbose = flags.count("-v");

  solver->solve(exclude, verbose);

  log(1, "c CPU time: %.2f seconds\n", SECONDS(clock() - begin));
  return 0;
}
