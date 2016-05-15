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



#ifdef __cplusplus
extern "C"{
#endif

typedef struct _id_bc_pair{
	char * id;
	double bc;

}id_bc_pair;

typedef void c_graph_parser;

c_graph_parser* new_graph_parser(int weight, int heuristic);

void graph_parser_parse_netjson(c_graph_parser* v, char *json);
void graph_parser_calculate_bc(c_graph_parser* v);
void graph_parser_compose_bc_map(c_graph_parser* v, id_bc_pair *map);
void graph_parser_parse_jsoninfo(c_graph_parser* v, char *json);
void delete_my_class(c_graph_parser* v);


#ifdef __cplusplus
}
#endif
