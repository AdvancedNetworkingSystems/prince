//
// Created by quynh on 12/15/15.
//

#ifndef GRAPH_PARSER_UTILITY_H
#define GRAPH_PARSER_UTILITY_H

#include <iostream>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_graph.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/graph/iteration_macros.hpp>
#include "common.h"



void printGraph(Graph &g);

namespace outops {
    std::ostream& operator<<(std::ostream& os, const Graph& g);

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::set<T>& s);
}

// non-member functions operating on Graph datatype.
namespace graphext {
    void id_of_vertices(const Graph& g, std::set<std::string>& r);

    template <typename Container>
    void id_of_vertices(const Graph& g, const Container& container, std::set<std::string>& r);
}



#include "utility.tpp"

#endif //GRAPH_PARSER_UTILITY_H

