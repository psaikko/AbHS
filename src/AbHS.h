#ifndef abhs_h
#define abhs_h

#include "AbdInstance.h"
#include "Util.h"
#include "MinisatInterface.h"
#include "CPLEXInterface.h"

#include <vector>
#include <unordered_map>
#include <iosfwd>

class AbHS {
 public:
  AbHS(AbdInstance& instance);
  ~AbHS();

  void solve(bool exclude_core, bool verbose);
  void printSolution(std::ostream & out);
  void setAssumptions(std::vector<int>& hs);
  void solveSAT(std::vector<int> & out_result, bool & issat, 
    std::vector<int> & hypothesis, bool check_manifestations);
  void unsatClauses(std::vector<int> & out_clauses, std::vector<int> & model,
    std::unordered_map<int, std::vector<int>*> & hypothesis);

  MinisatInterface* sat_solver;
  CPLEXInterface* mip_solver;
  AbdInstance &instance;
};

#endif