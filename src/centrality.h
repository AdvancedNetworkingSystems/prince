//
// Created by quynh on 12/15/15.
//

#ifndef GRAPH_PARSER_CENTRALITY_H
#define GRAPH_PARSER_CENTRALITY_H

#include <iostream>
#include <fstream>
#include <boost/graph/betweenness_centrality.hpp>
#include "common.h"

void simpleBetweennessCentrality(const Graph &g, string fileSuffix);
void writeBetweennessCentrality(const Graph &g, std::vector<double> v_centrality_vec, string fileSuffix);

#endif //GRAPH_PARSER_CENTRALITY_H
