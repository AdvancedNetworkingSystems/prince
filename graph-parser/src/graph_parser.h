/* Graph_Parser_lib.h*/

#include "../../prince/src/parser.h"
#ifdef __cplusplus
#include <cstdlib>
#include <iostream>
#include <string>
#include "centrality.h"
#include "graph_manager.h"
#include "betweenness_centrality.h"
#include "betweenness_centrality_heuristic.h"
#include "bc_interface.h"
#include "parser.h"

class graph_parser
{
public:

	graph_parser(bool weight, bool _heuristic);
	~graph_parser();
	void calculate_bc();
	void compose_bc_map(vector<pair<string, double> > & map);
	void compose_degree_map(vector<pair<string, int> > &map);
	void _parse_simplegraph(struct topology *topo);

	int get_n_edges();


private:
  GraphManager gm;
  BetweennessCentralityInterface *bci;
  bool heuristic;
};



extern "C"{
#endif



typedef void c_graph_parser;

c_graph_parser* new_graph_parser(int weight, int heuristic);
void graph_parser_parse_simplegraph(c_graph_parser* v, struct topology *topo);
void graph_parser_calculate_bc(c_graph_parser* v);
int graph_parser_compose_degree_bc_map(c_graph_parser* v, map_id_degree_bc *map);
void delete_graph_parser(c_graph_parser* v);

#ifdef __cplusplus
}
#endif
