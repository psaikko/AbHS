#ifndef wcnf_parser_h
#define wcnf_parser_h

#include <vector>
#include <iosfwd>

void parseWCNF(std::istream & wcnf_in, std::vector<double>& out_weights, double& out_top, 
               std::vector<int>& out_manifestations,
               std::vector<std::vector<int> >& out_clauses);
 
#endif