// Graph_Parser_lib.h

#include <cstdlib>
#include <iostream>
#include <string>
#include "centrality.h"
#include "graph_manager.h"
#include "betweenness_centrality.h"
#include "betweenness_centrality_heuristic.h"

class graph_parser
{
public:

	graph_parser(bool weight=true, bool heuristic=false);
	void _parse_netjson(std::basic_istream<char> &istream);
	void calculate_bc();
	void compose_bc_map(vector<pair<string, double> > & map);


private:
  GraphManager gm;
  BetweennessCentrality bc;
  BetweennessCentralityHeuristic bcc;
  bool heuristic;
};
