/* Graph_Parser_lib.h*/

#include "../../prince/src/parser.h"


typedef void c_graph_parser;

c_graph_parser* new_graph_parser(int weight, int heuristic);
void graph_parser_parse_simplegraph(c_graph_parser* v,struct topology *topo);
void graph_parser_calculate_bc(c_graph_parser* v);
int graph_parser_compose_degree_bc_map(c_graph_parser* v, map_id_degree_bc *map);
void delete_graph_parser(c_graph_parser* v);
