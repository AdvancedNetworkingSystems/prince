/*
 * grap_parser_wrapper.h
 *
 *  Created on: 10 mag 2016
 *      Author: gabriel
 */

#ifndef SRC_GRAPH_PARSER_GRAPH_PARSER_WRAPPER_H_
#define SRC_GRAPH_PARSER_GRAPH_PARSER_WRAPPER_H_

#include <iostream>
#include <istream>
#include <streambuf>
#include <string>
#include "graph_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

struct membuf : std::streambuf
{
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

typedef struct graph_parser graph_parser;

graph_parser* new_graph_parser(int weight, int heuristic);

void graph_parser_parse_netjson(graph_parser* v, char *json);
void graph_parser_calculate_bc(graph_parser* v);
void graph_parser_compose_bc_map(graph_parser* v, char *map);

void deleteMyClass(graph_parser* v);

#ifdef __cplusplus
}
#endif

#endif /* SRC_GRAPH_PARSER_GRAPH_PARSER_WRAPPER_H_ */
