//
// Created by quynh on 12/13/15.
//

#ifndef GRAPH_PARSER_PARSER_H
#define GRAPH_PARSER_PARSER_H

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include "common.h"
#include "graph_manager.h"

void readEdgeFileGraphManager(string filepath, GraphManager& gm);
void readJsonGraphManager(string filepath, GraphManager & gm);
void readComplexJsonGraphManager(string filepath, GraphManager& gm);

#endif //GRAPH_PARSER_PARSER_H


