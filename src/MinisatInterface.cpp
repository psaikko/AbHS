#include "MinisatInterface.h"
#include "Util.h"
#include <algorithm> // count, sort
#include <cstdio>
using namespace std;

MinisatInterface::MinisatInterface() { 
  minisat = new Minisat::Solver(); 

  //setFPU();
}

void MinisatInterface::addVariable(int var) 
{
  while (abs(var) >= minisat->nVars()) 
    newVar();
}
 
// solve instance and get unsatisfiable subformula (core)
void MinisatInterface::findCore(vector<int>& out_core) 
{
  out_core.clear();
  solve();
  getConflict(out_core);
}

// solve instance with current assumptions, return SAT?
bool MinisatInterface::solve() 
{
  bool ret;

  ret = minisat->solve(assumptions);

  return ret;
}

void MinisatInterface::getConflict(vector<int>& out_core) 
{
  const Minisat::vec<Minisat::Lit>& conflict = minisat->conflict.toVec();
  int conflict_size = conflict.size();

  //log(2, conflict_size ? "c Core found\n" : "c No core found\n");

  out_core.clear();
  out_core.resize(conflict_size);
  for (int i = 0; i < conflict_size; i++) {
    int v = Minisat::var(conflict[i]);
    int s = Minisat::sign(conflict[i]);
    out_core[i] = v * (1 - 2 * s);
  }
}

int MinisatInterface::newVar() {
  int v = 0;
  do {
    v = minisat->newVar(upol);
  } while (v == 0);
  return v;
}

void MinisatInterface::addConstraint(vector<int>& constr) 
{
  Minisat::vec<Minisat::Lit> minisat_clause(constr.size());
  for (unsigned i = 0; i < constr.size(); ++i) {
    int v = abs(constr[i]);
    bool s = constr[i] < 0;
    Minisat::Lit l = s ? ~Minisat::mkLit(v) : Minisat::mkLit(v);
    minisat_clause[i] = l;
  }
  minisat->addClause_(minisat_clause);
}

void MinisatInterface::getModel(vector<bool>& model) 
{
  model.clear();
  int model_size = minisat->model.size();
  model.resize(model_size);

  for (int i = 0; i < model_size; ++i)
    model[i] = (minisat->model[i] == Minisat::l_True);
}

double MinisatInterface::getModelWeight(std::unordered_map<int, double>& bvar_weights) {

  condTerminate(minisat->model.size() == 0, 1, "MinisatInterface::getModelWeight: no model exists\n");

  double opt = 0;
  for (auto kv : bvar_weights)
    if (minisat->model[kv.first] == Minisat::l_True) opt += kv.second;
  return opt;
}