//
// Created by quynh on 12/13/15.
//

#ifndef GRAPH_PARSER_COMMON_H
#define GRAPH_PARSER_COMMON_H

#include <iostream>
#include <cstdlib>
#include <exception>
#include <string>
#include <set>
#include <list>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>

using namespace std;

class Router { // aka Vertex
public:
    string id;
    string name;

    Router() { };
    Router(string id, string name) : id(id), name(name) { };

    bool operator<(const Router& rhs)
    {
       return id < rhs.id;
    }
   bool operator==(const Router& rhs)
   {
       return id == rhs.id;
   }
};

struct Link { // aka Edge
    double cost;

    Link() { };
    Link(double cost) : cost(cost) { };
};

// List typedefs
typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS,
        Router, Link> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::graph_traits<Graph>::vertex_iterator Viter;
typedef std::pair<Viter, Viter> Vpair;

typedef boost::graph_traits<Graph>::edge_descriptor Edge;
typedef boost::graph_traits<Graph>::edge_iterator Eiter;
typedef std::pair<Eiter, Eiter> Epair;
typedef boost::graph_traits<Graph>::edges_size_type edges_size_type;

typedef map<Vertex, size_t> StdVertexIndexMap;
// This property map is an adaptor that converts any type that is a model of Unique Associative Container such as std::map into a mutable Lvalue Property Map.
typedef boost::associative_property_map<StdVertexIndexMap> VertexIndexMap;

typedef map<Edge, size_t> StdEdgeIndexMap;
typedef boost::associative_property_map<StdEdgeIndexMap> EdgeIndexMap;


typedef std::vector<Vertex> VertexVec;
typedef std::vector<Vertex>::iterator VertexVecIter;

typedef std::map<Vertex, int> VertexMap;
typedef std::map<Vertex, int>::iterator VertexMapIter;


typedef std::vector<double> CentralityVec;
typedef boost::iterator_property_map<CentralityVec::iterator, VertexIndexMap> CentralityMap;

#endif //GRAPH_PARSER_COMMON_H


