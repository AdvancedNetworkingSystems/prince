// Graph_Parser_lib.h

#include <cstdlib>
#include <iostream>
#include <string>
#include "centrality.h"
#include "graph_manager.h"
#include "betweenness_centrality.h"
#include "betweenness_centrality_heuristic.h"
#include "parser.h"


class graph_parser
{
public:

	graph_parser(bool weight=true, bool heuristic=false);
	void _parse_netjson(std::basic_istream<char> &istream);
	void calculate_bc();
	void compose_bc_map(vector<pair<string, double> > & map);
	void _parse_jsoinfo(std::basic_istream<char> &istream);

private:
  GraphManager gm;
  BetweennessCentrality bc;
  BetweennessCentralityHeuristic bcc;
  bool heuristic;
};
