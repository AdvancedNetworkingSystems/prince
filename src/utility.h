//
// Created by quynh on 12/15/15.
// All the generic functions are defined in utility.tpp - 27/01/2015
//

#ifndef GRAPH_PARSER_UTILITY_H
#define GRAPH_PARSER_UTILITY_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_graph.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include "common.h"

namespace outops {
    std::ostream& operator<<(std::ostream& os, const Graph& g);

    // I have to use pair to add more than one argument for cout operator<<
    std::ostream& operator<<(std::ostream& os, std::pair<const Graph&, const VertexIndexPMap&> p);

    // For set
    template <typename T> std::ostream& operator<<(std::ostream& os, const std::set<T>& data);

    // For vector
    template <typename T> std::ostream& operator<<(std::ostream& os, const std::vector<T>& data);

    // For map
    template <typename T> std::ostream& operator<<(std::ostream& os, const std::map<string, T>& data);

    // For 2-D vector
    std::ostream& operator<<(std::ostream& os, const vector< vector< int> >& data);
}

namespace printhelper {
    template <typename T1, typename T2> void for_map(const std::map<T1, T2> m);
}

// non-member functions operating on Graph datatype.
template < typename TimeMap, typename T > class bfs_time_visitor : public boost::default_bfs_visitor {
public:
    bfs_time_visitor(TimeMap tmap, T & t):m_timemap(tmap), m_time(t) { }
    template < typename Vertex, typename Graph >
    void discover_vertex(Vertex u, const Graph & g) const
    {
        boost::put(m_timemap, u, m_time++);
    }
    TimeMap m_timemap;
    T & m_time;
};

namespace graphext {
    void id_of_all_vertices(const Graph& g, std::set<std::string>& r);

    template <typename Container>
    void id_of_some_vertices(const Graph& g, const Container& container, std::set<std::string>& r);

    bool is_connected(const Graph& g, const VertexIndexPMap& v_index_pmap);

    void print_edge(const Graph& g, const Edge& e);

    void print_v_index_std_map(const Graph& g, const VertexIndexStdMap& v_index_std_map);
    void print_v_index_pmap(const Graph& g, const VertexIndexPMap& v_index_pmap);
    void print_e_index_pmap(const Graph& g, const EdgeIndexPMap& e_index_pmap);

    void write_betweenness_centrality(Graph const& g, std::vector<double> v_centrality_vec, string filepath);
}

namespace setops {
    template <typename T> std::set<T> operator-(const std::set<T>& a, const std::set<T>& b);
    template <typename T> std::set<T> operator/(const std::set<T>& a, const std::set<T>& b);
}

namespace stdhelper {
    template <typename T1, typename T2> bool exists(const std::map<T1, T2>& m, const T1& key);
    template <typename T> bool exists(const std::set<T>& s, const T& element);

    template <typename T>
    std::string to_string(T value);
}

namespace helper {
    // I do not want to use boost::filesystem, due to additional library must be included
    void get_file_name_and_extension(string s, string& name, string& ext);
}

template <typename Pair>
struct second_equal_to
    : std::unary_function<const Pair&, bool>
{
    second_equal_to(const typename Pair::second_type& value)
        : value_(value) { }

    bool operator()(const Pair& p) const
    {
        // I quick-hacked to make the Pair work for second argument with built-in type, such as int.
        // return p.second == *value_;
        return p.second == value_;
    }

private:
    typename Pair::second_type value_;
};

template <typename T1, typename T2>
struct less_second {
    typedef pair<T1, T2> type;
    bool operator ()(type const& a, type const& b) const {
        return a.second < b.second;
    }
};

#include "utility.tpp"

#endif //GRAPH_PARSER_UTILITY_H

