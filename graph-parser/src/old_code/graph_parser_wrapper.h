/*
 * grap_parser_wrapper.h
 *
 *  Created on: 10 mag 2016
 *      Author: gabriel
 */

#ifndef SRC_GRAPH_PARSER_GRAPH_PARSER_WRAPPER_H_
#define SRC_GRAPH_PARSER_GRAPH_PARSER_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>


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

#endif /* SRC_GRAPH_PARSER_GRAPH_PARSER_WRAPPER_H_ */
