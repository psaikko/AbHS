#ifndef cplex_solver_h
#define cplex_solver_h

#include <ilcplex/ilocplex.h>
#include <vector>
#include <unordered_map>

class CPLEXInterface {

 public:
  CPLEXInterface();
  ~CPLEXInterface();
  void addObjectiveVariable(int bVar, double weight);
  void addObjectiveVariables(std::unordered_map<int, double> & bvar_weights);
  void reset();
  void forbidCurrentSolution();
  void addConstraint(std::vector<int>& core, bool inc = true);

  bool computeHS(std::vector<int>& hittingSet, double& weight);

 private:

  IloNumVar newObjVar(int v);

  bool solutionExists;
  bool objFuncAttached;

  IloEnv* env;
  IloModel model;
  IloNumVarArray objVars;
  IloNumVarArray vars;
  unsigned nObjVars;
  unsigned nVars;
  IloObjective objective;
  IloRangeArray cons;
  IloCplex cplex;
  std::unordered_map<int, IloNumVar> var_to_IloVar;
};

#endif