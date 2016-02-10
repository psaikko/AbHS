#include "CPLEXInterface.h"
#include "Util.h"

CPLEXInterface::CPLEXInterface() : solutionExists(false), objFuncAttached(false), nObjVars(0), nVars(0) {

  env = new IloEnv();
  model = IloModel(*env);
  objVars = IloNumVarArray(*env);
  vars    = IloNumVarArray(*env);
  cons = IloRangeArray(*env);
  cplex = IloCplex(model);
  cplex.setParam(IloCplex::Threads, 1);
  cplex.setParam(IloCplex::EpGap, 0.0);
  cplex.setParam(IloCplex::EpAGap, 0.0);
  cplex.setParam(IloCplex::MIPDisplay, 0);

  objective = IloMinimize(*env, IloExpr(*env));
}

CPLEXInterface::~CPLEXInterface() {
  (*env).end();
  delete env;
}

IloNumVar CPLEXInterface::newObjVar(int bVar) {
  //log(2, "c MIP obj variable %d %.2f\n", bVar, w);
  char name[11];
  snprintf(name, 11, "%10d", bVar);

  IloNumVar x(*env, 0.0, 1.0, ILOBOOL, name);
  var_to_IloVar[bVar] = x;
  objVars.add(x);
  nObjVars++;
  return x;
}

// Create a CPLEX variable for the bvar and append
// it to the objective function with the given weight
void CPLEXInterface::addObjectiveVariable(int bVar, double weight) {
  if (objFuncAttached) {
    model.remove(objective);
    objFuncAttached = false;
  }

  objective.setExpr(objective.getExpr() + weight * newObjVar(bVar));
}

// adds variables to the cplex instance and sets the objective function to
// minimize weight
void CPLEXInterface::addObjectiveVariables(std::unordered_map<int, double> & bvar_weights) {
  if (objFuncAttached) {
    model.remove(objective);
    objFuncAttached = false;
  }

  IloNumExpr objExpr = objective.getExpr();

  double total_w = 0;

  for (auto b_w : bvar_weights) {
    objExpr += b_w.second * newObjVar(b_w.first);
    total_w += b_w.second;
  }

  objective.setExpr(objExpr);

  if (total_w >= 10000000000) {
    // turn on extra precision --
    printf("c increase CPLEX abs. tolerances for very high weights\n");
    //cplex.setParam(IloCplex::EpInt, 0.01);
    cplex.setParam(IloCplex::EpAGap, 1e-4);
    cplex.setParam(IloCplex::EpOpt, 1e-4);
    cplex.setParam(IloCplex::EpRHS, 1e-4);
    cplex.setParam(IloCplex::EpLin, 1e-4);
    cplex.setParam(IloCplex::EpMrk, 0.02);
    cplex.setParam(IloCplex::NumericalEmphasis, true);
  }
}

void CPLEXInterface::reset() {
  model.remove(objective);
  objective.end();
  objective = IloMinimize(*env, IloExpr(*env));
  
  model.remove(objVars);
  objVars.endElements();
  objVars.end();
  objVars = IloNumVarArray(*env);
  nObjVars = 0;

  cons.endElements();
  cons.end();
  cons = IloRangeArray(*env);

  var_to_IloVar.clear();
  objFuncAttached = false;
  solutionExists = false;
}

// disallow a found solution hs by adding a constraints to the MIP instance
void CPLEXInterface::forbidCurrentSolution() {
  condTerminate(!solutionExists, 1, 
    "CPLEXInterface::forbidCurrentSolution - no current hitting set exists\n");

  IloExpr sub_expr(*env);  // make sure next solution isn't a subset of this one
  IloExpr super_expr(*env);  // and not a superset

  // get values from cplex
  IloNumArray vals(*env);
  cplex.getValues(vals, objVars);

  double solutionSize = 0;

  // build constraints of clauses in current solution and those not in current
  // solution
  for (unsigned i = 0; i < nObjVars; ++i) {
    if (IloAbs(vals[i] - 1.0) < EPS) {
      super_expr += objVars[i];
      ++solutionSize;
    } else {
      sub_expr += objVars[i];
    }
  }

  // include at least one clause not in this solution
  IloRange sub_con = (sub_expr >= 1.0);
  // leave out at least one clause in this solution
  IloRange super_con = (super_expr <= (solutionSize - 1));

  cons.add(sub_con);
  cons.add(super_con);
  model.add(sub_con);
  model.add(super_con);
}

// adds a constraint to the MIP instance, works with clauses containing negative
// literals
void CPLEXInterface::addConstraint(std::vector<int>& core, bool inc) {
  condTerminate(core.empty(), 1, 
    "CPLEXInterface::addConstraint - empty constraint\n");

  IloExpr expr(*env);
  for (int c : core) {
    IloNumVar x = var_to_IloVar[abs(c)];
    if (c > 0) {
      expr += x;
    } else {
      expr += -x;
    }
  }
  IloRange con = inc ? (expr >= 1.0) : (expr <= (int)core.size() - 1);
  cons.add(con);
  model.add(con);
}

// returns positive valued objective function variables in 'solution'
// and value of objective function in 'weight'
bool CPLEXInterface::computeHS(std::vector<int>& hittingSet, double& weight) {
  if (!objFuncAttached) {
    model.add(objective);
    objFuncAttached = true;
  }

  //log(2, "c CPLEX: solving MIP problem\n");

  hittingSet.clear();
  if (!cplex.solve()) return false;

  //log(2, "c CPLEX: Solution status = %d\n", cplex.getStatus());
  log(2, "c CPLEX: Solution value = %f\n", cplex.getObjValue());

  // get objective variable values from cplex
  IloNumArray vals(*env);
  cplex.getValues(vals, objVars);

  // gather set of variables with true value
  for (unsigned i = 0; i < nObjVars; ++i) {
    if (IloAbs(vals[i] - 1.0) < EPS) {
      // parse bvars from cplex variable names
      int bVar;
      sscanf(objVars[i].getName(), "%100d", &bVar);
      hittingSet.push_back(bVar);
    }
  }

  //log(2, "c CPLEX: hitting set:\n");
  //logVec(2, hittingSet);

  weight = cplex.getObjValue();
  solutionExists = true;
  return true;
}
