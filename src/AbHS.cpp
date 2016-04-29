#include "AbHS.h"
#include "Util.h"

#include <ostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <unordered_set>
#include <unordered_map>

#include <errno.h>
#include <math.h>  // ceil
#include <assert.h>

using namespace std;

AbHS::AbHS(AbdInstance &inst)
    : sat_solver(nullptr),
      mip_solver(nullptr),
      instance(inst)
{
}

AbHS::~AbHS() {
  delete mip_solver;
  delete sat_solver;
}

void AbHS::solveSAT(vector<int> & out_result, bool & issat,
  vector<int> & hypothesis, bool check_manifestations) {

    sat_solver->deactivateClauses();
    for (int c : hypothesis) sat_solver->activateClause(c);
    if (check_manifestations)
      sat_solver->activateClause(instance.manifestation_bvar);
    else
      sat_solver->deactivateClause(instance.manifestation_bvar);

    sat_solver->findCore(out_result);
    if (!out_result.empty()) {
      issat = false;
      //log(0, "c Core:\n");
      //logVec(0, out_result);
    } else {
      issat = true;
      instance.getModel(out_result);
      //log(0, "c Model:\n");
      //logVec(0, out_result);
    }
}

void AbHS::unsatClauses(vector<int> & out_clauses, vector<int> & model,
  unordered_map<int, vector<int>*> & hypothesis) {

  out_clauses.clear();
  // find all hypothesis clauses not satisfied by C
  // k <- {h \in H | C(h) = 0}
  for (auto p : hypothesis) {
    int bvar = p.first;
    vector<int> clause = *p.second;
    for (int l : clause) {
      if (count(model.begin(), model.end(), l)) goto skip_clause;
    }
    out_clauses.push_back(bvar);
    skip_clause: continue;
  }
}

void AbHS::solve(bool exclude_core, bool verbose) {

  sat_solver = new MinisatInterface();
  mip_solver = new CPLEXInterface();

  instance.attach(sat_solver);
  instance.attach(mip_solver);

  vector<int> solution;
  double weight = 0;
  bool issat = false;

  vector<int> k;
  vector<int> S;
  vector<int> C;
  unordered_map<int, vector<int>*> H = instance.bvar_hypothesis;

  if (verbose) {
    log(0, "c Theory:\n");
    for (auto clause : instance.theory) logVec(0, *clause);
    log(0, "c Hypothesis:\n");
    for (auto clause : instance.hypothesis) logVec(0, *clause);
    log(0, "c Manifestations:\n");
    logVec(0, instance.manifestations);
  }

  for (int iter = 0;;++iter) {
    if (verbose) log(0, "c\nc iteration %d\n", iter);
    bool ok = mip_solver->computeHS(S, weight);

    if (!ok) {
      log(0, "c no hitting set\n");
      break;
    }

    if (verbose) {
      log(0, "c MCHS:\n");
      logVec(0, S);
    }
    log(0, "c lb %f\n", weight);

    solveSAT(C, issat, S, true);

    if (issat) {
      unsatClauses(k, C, H);

      if (k.empty()) {
        log(0, "c Empty abduction core\n");
        break;
      }
      if (verbose) {
        log(0, "c AbdCore1:\n");
        logVec(0, k);
      }
      mip_solver->addConstraint(k);
      continue;
    }

    solveSAT(C, issat, S, false);

    if (!issat) {

      k.clear();
      // k <- H \ S
      for (auto p : H) {
        int bvar = p.first;
        if (!count(S.begin(), S.end(), bvar)) k.push_back(bvar);
      }

      if (k.empty()) {
        if (verbose) log(0, "c Empty abduction core\n");
        break;
      }

      if (exclude_core) {
        if (verbose) {
          log(0, "c reduced size by %ld\n", (long)k.size() - (long)C.size());
          log(0, "c AbdCore2:\n");
          logVec(0, C);
        }
        mip_solver->addConstraint(C, false);
      } else {
        if (verbose) {
          log(0, "c AbdCore2:\n");
          logVec(0, k);
        }
        mip_solver->addConstraint(k, true);
      }
      continue;
    }

    log(0, "c OPT\n");
    // logVec(0, S);
    log(0, "c hypothesis:\n");
    for (auto s : S)
      logVec(0, *H[s]);
    log(0, "o %f\n", weight);
    return;
  }

  log(0, "c NO SOLUTION\n");
}
