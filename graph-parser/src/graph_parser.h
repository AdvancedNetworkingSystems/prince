/* Graph_Parser_lib.h*/


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
	void _parse_netjson(std::basic_istream<char> &istream);
	void calculate_bc();
	void compose_bc_map(vector<pair<string, double> > & map);
	void compose_degree_map(vector<pair<string, int> > &map);
	void _parse_jsoninfo(std::basic_istream<char> &istream);
	int get_n_edges();
	string get_originator();


private:
  GraphManager gm;
  BetweennessCentralityInterface *bci;
  bool heuristic;
};



extern "C"{
#endif


typedef struct _id_bc_pair{
	char* id;
	double bc;

}id_bc_pair;

typedef struct _map_id_bc_pair{
	id_bc_pair *map;
	size_t size;
}map_id_bc_pair;

typedef struct _id_degree_pair{
	char* id;
	int degree;

}id_degree_pair;

typedef struct _map_id_degree_pair{
	id_degree_pair *map;
	size_t size;
	int n_edges;
	char *originator;
}map_id_degree_pair;


typedef void c_graph_parser;

c_graph_parser* new_graph_parser(int weight, int heuristic);

void graph_parser_parse_netjson(c_graph_parser* v, char *json);
void graph_parser_calculate_bc(c_graph_parser* v);
void graph_parser_compose_bc_map(c_graph_parser* v, map_id_bc_pair *map);
void graph_parser_compose_degree_map(c_graph_parser* v, map_id_degree_pair * map);
void graph_parser_parse_jsoninfo(c_graph_parser* v, char *json);
void delete_graph_parser(c_graph_parser* v);

#ifdef __cplusplus
}
#endif
