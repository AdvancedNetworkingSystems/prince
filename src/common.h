//
// Created by quynh on 12/13/15.
//

#ifndef GRAPH_PARSER_COMMON_H
#define GRAPH_PARSER_COMMON_H

#include <string>
#include <array>
using namespace std;

struct Router {
    string id;
    string name;

    Router() { };

    Router(string id, string name) : id(id), name(name) { }
};

struct Link {
    double cost;

    Link() { };

    Link(double cost) : cost(cost) { };
};

// List typedefs
typedef array<string, 3> graphDataType;
typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS,
        Router, Link> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::graph_traits<Graph>::vertex_iterator Viter;

typedef boost::graph_traits<Graph>::edge_descriptor Edge;

#endif //GRAPH_PARSER_COMMON_H


