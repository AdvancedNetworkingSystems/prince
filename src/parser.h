//
// Created by quynh on 12/13/15.
//

#ifndef GRAPH_PARSER_PARSER_H
#define GRAPH_PARSER_PARSER_H

#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "common.h"
using namespace std;

vector<graphDataType> readEdgeFile(string filePath);

#endif //GRAPH_PARSER_PARSER_H


