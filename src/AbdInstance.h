#ifndef AbdInstance_h
#define AbdInstance_h

#include "MinisatInterface.h"
#include "CPLEXInterface.h"
#include "Util.h"

#include <unordered_map>
#include <vector>
#include <iosfwd>

class AbdInstance {
 public:
  AbdInstance(std::istream & wcnf_in);
  ~AbdInstance();

  std::unordered_map<int, double> bvar_weights;

  std::vector<std::vector<int>*> clauses;
  std::vector<std::vector<int>*> theory;
  std::vector<std::vector<int>*> hypothesis;
  std::vector<int> manifestations;
  std::unordered_map<int, bool> isOriginalVariable;

  std::unordered_map<int, std::vector<int>*> bvar_hypothesis;

  void addToTheory(std::vector<int>& clause);
  int addToHypothesis(std::vector<int>& clause, double weight);
  int setManifestations(std::vector<int>& manifestations);

  void getModel(std::vector<int>& out_model);
  double getModelWeight();

  double totalWeight();

  void printStats();

  void attach(MinisatInterface * solver);
  void attach(CPLEXInterface * solver);

  int max_var; 
  int manifestation_bvar;
  clock_t parseTime;

 private:
  AbdInstance(const AbdInstance&);
  void operator=(AbdInstance const&);

  void init();

  void updateBvarMap(int bvar, std::vector<int>* clause);

  MinisatInterface* sat_solver;
  CPLEXInterface* mip_solver;

};

#endif