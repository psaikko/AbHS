#include "AbdInstance.h"
#include "WCNFParser.h"
#include <istream>
#include <algorithm>
#include <zlib.h>

using namespace std;

void AbdInstance::attach(MinisatInterface* s) {
  sat_solver = s;

  // add all variables to solver
  for (int i = 0; i <= max_var; ++i) sat_solver->addVariable(i);

  // create assumption data for bvars
  for (auto kv : bvar_weights) sat_solver->addBvarAssumption(kv.first);
  sat_solver->addBvarAssumption(manifestation_bvar);

  // if user-specified branching for sat solver, set decision vars

  for (auto cl : clauses) sat_solver->addConstraint(*cl);
}

void AbdInstance::attach(CPLEXInterface* s) {
  mip_solver = s;
  mip_solver->addObjectiveVariables(bvar_weights);
}


AbdInstance::AbdInstance(std::istream& wcnf_in) : 
  max_var(0), parseTime(0), sat_solver(nullptr), mip_solver(nullptr)
{

  clock_t start = clock();

  condTerminate(wcnf_in.fail(), 1,
                "Error: Bad input stream (check filepath)\n");

  double out_top;
  vector<double> out_weights;
  vector<vector<int>> out_clauses;
  vector<int> out_manifestations;
  parseWCNF(wcnf_in, out_weights, out_top, out_manifestations, out_clauses);

  for (auto& cl : out_clauses)
    for (int i : cl) 
      max_var = max(abs(i), max_var);

  manifestation_bvar = setManifestations(out_manifestations);

  for (unsigned i = 0; i < out_clauses.size(); ++i) {
    if (out_weights[i] < out_top) {
      addToHypothesis(out_clauses[i], out_weights[i]);
    } else {
      addToTheory(out_clauses[i]);
    }
  }

  parseTime = (clock() - start);
}

AbdInstance::~AbdInstance() {
  for (auto p : clauses) delete p;
  for (auto p : bvar_hypothesis) delete p.second;
}

// add a hard clause to the SAT instance
void AbdInstance::addToTheory(vector<int>& hc) {

  for (int v : hc) {
    max_var = max(abs(v), max_var);
    if (sat_solver != nullptr && max_var >= sat_solver->nVars())
      sat_solver->addVariable(max_var);
  }

  auto clause = new vector<int>(hc);

  for (int l : *clause) isOriginalVariable[abs(l)] = true;

  theory.push_back(clause);
  clauses.push_back(clause);

  if (sat_solver != nullptr) sat_solver->addConstraint(hc);
}

// add a soft clause to the SAT instance
int AbdInstance::addToHypothesis(vector<int>& sc, double weight) {

  for (int v : sc) {
    max_var = max(abs(v), max_var);
    if (sat_solver != nullptr)
      while (max_var >= sat_solver->nVars()) sat_solver->addVariable(max_var);
  }

  auto clause = new vector<int>(sc);

  for (int l : *clause) isOriginalVariable[abs(l)] = true;

  int bVar = ++max_var;
  if (sat_solver != nullptr) sat_solver->addVariable(bVar);
  if (mip_solver != nullptr) mip_solver->addObjectiveVariable(bVar, weight);

  bvar_weights[bVar] = weight;
  bvar_hypothesis[bVar] = clause;
  
  auto b_clause = new vector<int>(sc);
  b_clause->push_back(bVar);

  hypothesis.push_back(b_clause);
  clauses.push_back(b_clause);

  if (sat_solver != nullptr) sat_solver->addBvarAssumption(bVar);

  return bVar;
}

int AbdInstance::setManifestations(std::vector<int>& ms) {

  manifestations = ms;

  for (int l : ms) {
    max_var = max(abs(l), max_var);
    if (sat_solver != nullptr)
      while (max_var >= sat_solver->nVars()) sat_solver->addVariable(max_var);
  }

  auto clause = new vector<int>();
  for (int l : ms) clause->push_back(-l);

  for (int l : *clause) isOriginalVariable[abs(l)] = true;

  int bVar = ++max_var;
  if (sat_solver != nullptr) sat_solver->addVariable(bVar);
  //if (mip_solver != nullptr) mip_solver->addObjectiveVariable(bVar, INT_MIN);

  //bvar_weights[bVar] = INT_MIN;

  clause->push_back(bVar);
  clauses.push_back(clause);

  if (sat_solver != nullptr) sat_solver->addBvarAssumption(bVar);

  return bVar;

}

void AbdInstance::getModel(std::vector<int>& out_solution) {

  condTerminate(sat_solver == nullptr, 1,
                "Error: no SAT solver attached to AbdInstance\n");

  out_solution.clear();
  vector<bool> model;
  sat_solver->getModel(model);

  condTerminate(model.size() == 0, 1, "Error: no model given by SAT solver\n");

  for (unsigned i = 0; i < model.size(); ++i) {
    if (isOriginalVariable[i]) {
      out_solution.push_back(model[i] ? i : -i);
    }
  }
}

double AbdInstance::getModelWeight() {

  condTerminate(sat_solver == nullptr, 1,
                "Error: no SAT solver attached to AbdInstance\n");

  return sat_solver->getModelWeight(bvar_weights);
}

double AbdInstance::totalWeight() {
  double sum = 0;
  for (auto v_w : bvar_weights) sum += v_w.second;
  return sum;
}