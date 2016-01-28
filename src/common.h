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

class Router { // aka VertexProperties
public:
    string id;
    string label;

    Router() { };
    Router(string i, string l) : id(i), label(l) { };

    bool operator<(const Router& rhs)
    {
       return id < rhs.id;
    }
   bool operator==(const Router& rhs)
   {
       return id == rhs.id;
   }
};

struct Link { // aka EdgeProperties
    double cost;

    Link() { };
    Link(double c) : cost(c) { };
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

typedef map<Vertex, size_t> VertexIndexStdMap;
// This property map is an adaptor that converts any type that is a model of Unique Associative Container such as std::map into a mutable Lvalue Property Map.
typedef boost::associative_property_map<VertexIndexStdMap> VertexIndexMap;

typedef map<Edge, size_t> EdgeIndexStdMap;
typedef boost::associative_property_map<EdgeIndexStdMap> EdgeIndexMap;

typedef std::vector<Vertex> VertexVec;
typedef std::vector<Vertex>::iterator VertexVecIter;

typedef std::map<Vertex, int> VertexMap;
typedef std::map<Vertex, int>::iterator VertexMapIter;

typedef std::map<std::string, int> NameToIntMap;

typedef std::vector<std::string> StringVec;
typedef std::vector<std::string>::iterator StringVecIter;

typedef std::set<std::string> StringSet;
typedef std::set<std::string>::iterator StringSetIter;

typedef std::vector<double> CentralityVec;
typedef boost::iterator_property_map<CentralityVec::iterator, VertexIndexMap> CentralityMap;

#endif //GRAPH_PARSER_COMMON_H


