//
// Created by quynh on 12/13/15.
//

#ifndef GRAPH_PARSER_PARSER_H
#define GRAPH_PARSER_PARSER_H

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include "common.h"

template<typename NameVertexMap>
void addLinkToGraph(string s1, string s2, double cost, Graph &g, NameVertexMap &routers);

void readEdgeFile(string filePath, Graph &g);
void readJson(string filePath, Graph &g);
void readComplexJson(string filePath, Graph &g);


#endif //GRAPH_PARSER_PARSER_H


