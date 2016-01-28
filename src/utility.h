//
// Created by quynh on 12/15/15.
// All the generic functions are defined in utility.tpp - 27/01/2015
//

#ifndef GRAPH_PARSER_UTILITY_H
#define GRAPH_PARSER_UTILITY_H

#include <iostream>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_graph.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/graph/iteration_macros.hpp>
#include "common.h"

namespace outops {
    std::ostream& operator<<(std::ostream& os, const Graph& g);

    // I have to use pair to add more than one argument for cout operator<<
    std::ostream& operator<<(std::ostream& os, std::pair<const Graph&, const VertexIndexMap&> p);

    template <typename T> // declared in utitlity.tpp
    std::ostream& operator<<(std::ostream& os, const std::set<T>& s);

    std::ostream& operator<<(std::ostream& os, const std::map<string, int>& m);

    std::ostream& operator<<(std::ostream& os, const vector< vector< int> >& data);
}

namespace printhelper {
    template <typename T1, typename T2> void for_map(const std::map<T1, T2> m);
}
// non-member functions operating on Graph datatype.
namespace graphext {
    void id_of_all_vertices(const Graph& g, std::set<std::string>& r);

    template <typename Container>
    void id_of_some_vertices(const Graph& g, const Container& container, std::set<std::string>& r);

    void print_v_index_std_map(const Graph& g, const VertexIndexStdMap& v_index_std_map);
    void print_v_index_map(const Graph& g, const VertexIndexMap& v_index_map);
}

namespace setops {
    template <typename T> std::set<T> operator-(const std::set<T>& a, const std::set<T>& b);
    template <typename T> std::set<T> operator/(const std::set<T>& a, const std::set<T>& b);
}

namespace stdhelper {
    template <typename T1, typename T2> bool exists(const std::map<T1, T2>& m, const T1& key);
    template <typename T> bool exists(const std::set<T>& s, const T& element);
}

template <typename Pair>
struct second_equal_to
    : std::unary_function<const Pair&, bool>
{
    second_equal_to(const typename Pair::second_type& value)
        : value_(value) { }

    bool operator()(const Pair& p) const
    {
        return p.second == value_;
    }

private:
    typename Pair::second_type value_;
};

#include "utility.tpp"

#endif //GRAPH_PARSER_UTILITY_H

