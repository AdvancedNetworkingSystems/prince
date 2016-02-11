//
// Created by quynh on 12/15/15.
// The code is mainly from boost/graph/betweenness_centrality.
// I modified the code to work with Traffic Matrix.
//

#ifndef GRAPH_PARSER_CENTRALITY_H
#define GRAPH_PARSER_CENTRALITY_H

#include <fstream>
#include <iostream>
#include <boost/graph/betweenness_centrality.hpp>
#include "common.h"

namespace boost {

namespace detail { namespace graph {

  template<typename Graph, typename TrafficMatrix,
    typename VertexDescriptor>
  int get_traffic_matrix_value(const Graph& g,
                              TrafficMatrix traffic_matrix,
                              VertexDescriptor v1,
                              VertexDescriptor v2) {
    string name_1 = g[v1].id;
    string name_2 = g[v2].id;
    std::pair<std::string, std::string> p;
    if (name_1.compare(name_2) <= 0)
    {
      p = std::pair<std::string, std::string>(name_1, name_2);
    }
    else {
      p = std::pair<std::string, std::string>(name_2, name_1);
    }

    // TODO: how to return the default value 1 for traffic_matrix.
    // If we are using std::map, then find() will be sufficient.
    // But we are using the PropertyMap from boost...
    // I don't think that there will be the case that p doesn't belong to traffic_matrix
    // But it might result in a bug if p does not exist in traffic_matrix
    int value = boost::get(traffic_matrix, p);
    return value;
  }

  template<typename Graph, typename CentralityMap, typename EdgeCentralityMap,
         typename TrafficMatrix,
         typename IncomingMap, typename DistanceMap,
         typename DependencyMap, typename PathCountMap,
         typename VertexIndexMap, typename ShortestPaths>
  void
  brandes_betweenness_centrality_heuristic_impl(const Graph& g,
                                      TrafficMatrix traffic_matrix,
                                      CentralityMap centrality,     // C_B
                                      EdgeCentralityMap edge_centrality_map,
                                      IncomingMap incoming, // P
                                      DistanceMap distance,         // d
                                      DependencyMap dependency,     // delta
                                      PathCountMap path_count,      // sigma
                                      VertexIndexMap vertex_index,
                                      ShortestPaths shortest_paths)
  {
    std::cout << "Heuristic Betweenness Centrality Implementation\n";

    typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
    typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;

    // Initialize centrality
    init_centrality_map(vertices(g), centrality);
    init_centrality_map(edges(g), edge_centrality_map);

    std::stack<vertex_descriptor> ordered_vertices;
    vertex_iterator s, s_end;
    for (boost::tie(s, s_end) = vertices(g); s != s_end; ++s) {
      // Initialize for this iteration
      vertex_iterator w, w_end;
      for (boost::tie(w, w_end) = vertices(g); w != w_end; ++w) {
        incoming[*w].clear();
        put(path_count, *w, 0);
        put(dependency, *w, 0);
      }
      put(path_count, *s, 1);

      // Execute the shortest paths algorithm. This will be either
      // Dijkstra's algorithm or a customized breadth-first search,
      // depending on whether the graph is weighted or unweighted.
      shortest_paths(g, *s, ordered_vertices, incoming, distance,
                     path_count, vertex_index);

      while (!ordered_vertices.empty()) {
        vertex_descriptor w = ordered_vertices.top();
        ordered_vertices.pop();

        typedef typename property_traits<IncomingMap>::value_type
          incoming_type;
        typedef typename incoming_type::iterator incoming_iterator;
        typedef typename property_traits<DependencyMap>::value_type
          dependency_type;

        // get communication intensity from traffic_matrix
        // dependency_type communication_intensity = dependency_type(1);
        dependency_type communication_intensity = get_traffic_matrix_value(g, traffic_matrix, w, *s);
        put(dependency, w, communication_intensity + get(dependency, w));

        dependency_type factor = dependency_type(get(dependency, w))
          / dependency_type(get(path_count, w));

        for (incoming_iterator vw = incoming[w].begin();
             vw != incoming[w].end(); ++vw) {
          vertex_descriptor v = source(*vw, g);

          dependency_type new_dependency_value = get(dependency, v) + (
            dependency_type(get(path_count, v)) * factor);
          put(dependency, v, new_dependency_value);
          update_centrality(edge_centrality_map, *vw, factor);
        }

        if (w != *s) {
          update_centrality(centrality, w, get(dependency, w));
        }
      }
    }

    typedef typename graph_traits<Graph>::directed_category directed_category;
    const bool is_undirected =
      is_convertible<directed_category*, undirected_tag*>::value;
    if (is_undirected) {
      divide_centrality_by_two(vertices(g), centrality);
      divide_centrality_by_two(edges(g), edge_centrality_map);
    }
  }



} } // end of namespace detail::graph

template<typename Graph,
         typename TrafficMatrix,
         typename CentralityMap, typename EdgeCentralityMap,
         typename IncomingMap, typename DistanceMap,
         typename DependencyMap, typename PathCountMap,
         typename VertexIndexMap>
void
brandes_betweenness_centrality_heuristic(const Graph& g,
                               TrafficMatrix traffic_matrix,
                               CentralityMap centrality,     // C_B
                               EdgeCentralityMap edge_centrality_map,
                               IncomingMap incoming, // P
                               DistanceMap distance,         // d
                               DependencyMap dependency,     // delta
                               PathCountMap path_count,      // sigma
                               VertexIndexMap vertex_index
                               BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph,vertex_list_graph_tag))
{
  detail::graph::brandes_unweighted_shortest_paths shortest_paths;

  detail::graph::brandes_betweenness_centrality_heuristic_impl(g,
                                                     traffic_matrix,
                                                     centrality,
                                                     edge_centrality_map,
                                                     incoming, distance,
                                                     dependency, path_count,
                                                     vertex_index,
                                                     shortest_paths);
}

template<typename Graph,
         typename TrafficMatrix,
         typename CentralityMap, typename EdgeCentralityMap,
         typename IncomingMap, typename DistanceMap,
         typename DependencyMap, typename PathCountMap,
         typename VertexIndexMap, typename WeightMap>
void
brandes_betweenness_centrality_heuristic(const Graph& g,
                               TrafficMatrix traffic_matrix,
                               CentralityMap centrality,     // C_B
                               EdgeCentralityMap edge_centrality_map,
                               IncomingMap incoming, // P
                               DistanceMap distance,         // d
                               DependencyMap dependency,     // delta
                               PathCountMap path_count,      // sigma
                               VertexIndexMap vertex_index,
                               WeightMap weight_map
                               BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph,vertex_list_graph_tag))
{
  detail::graph::brandes_dijkstra_shortest_paths<WeightMap>
    shortest_paths(weight_map);

  detail::graph::brandes_betweenness_centrality_heuristic_impl(g,
                                                     traffic_matrix,
                                                     centrality,
                                                     edge_centrality_map,
                                                     incoming, distance,
                                                     dependency, path_count,
                                                     vertex_index,
                                                     shortest_paths);
}

namespace detail { namespace graph {
  template<typename Graph,
           typename TrafficMatrix,
           typename CentralityMap, typename EdgeCentralityMap,
           typename WeightMap, typename VertexIndexMap>
  void
  brandes_betweenness_centrality_heuristic_dispatch2(const Graph& g,
                                           TrafficMatrix traffic_matrix,
                                           CentralityMap centrality,
                                           EdgeCentralityMap edge_centrality_map,
                                           WeightMap weight_map,
                                           VertexIndexMap vertex_index)
  {
    typedef typename graph_traits<Graph>::degree_size_type degree_size_type;
    typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;
    typedef typename mpl::if_c<(is_same<CentralityMap,
                                        dummy_property_map>::value),
                                         EdgeCentralityMap,
                               CentralityMap>::type a_centrality_map;
    typedef typename property_traits<a_centrality_map>::value_type
      centrality_type;

    typename graph_traits<Graph>::vertices_size_type V = num_vertices(g);

    std::vector<std::vector<edge_descriptor> > incoming(V);
    std::vector<centrality_type> distance(V);
    std::vector<centrality_type> dependency(V);
    std::vector<degree_size_type> path_count(V);

    brandes_betweenness_centrality_heuristic(
      g,
      traffic_matrix,
      centrality, edge_centrality_map,
      make_iterator_property_map(incoming.begin(), vertex_index),
      make_iterator_property_map(distance.begin(), vertex_index),
      make_iterator_property_map(dependency.begin(), vertex_index),
      make_iterator_property_map(path_count.begin(), vertex_index),
      vertex_index,
      weight_map);
  }


  template<typename Graph,
           typename TrafficMatrix,
           typename CentralityMap, typename EdgeCentralityMap,
           typename VertexIndexMap>
  void
  brandes_betweenness_centrality_heuristic_dispatch2(const Graph& g,
                                           TrafficMatrix traffic_matrix,
                                           CentralityMap centrality,
                                           EdgeCentralityMap edge_centrality_map,
                                           VertexIndexMap vertex_index)
  {
    typedef typename graph_traits<Graph>::degree_size_type degree_size_type;
    typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;
    typedef typename mpl::if_c<(is_same<CentralityMap,
                                        dummy_property_map>::value),
                                         EdgeCentralityMap,
                               CentralityMap>::type a_centrality_map;
    typedef typename property_traits<a_centrality_map>::value_type
      centrality_type;

    typename graph_traits<Graph>::vertices_size_type V = num_vertices(g);

    std::vector<std::vector<edge_descriptor> > incoming(V);
    std::vector<centrality_type> distance(V);
    std::vector<centrality_type> dependency(V);
    std::vector<degree_size_type> path_count(V);

    brandes_betweenness_centrality_heuristic(
      g,
      traffic_matrix,
      centrality, edge_centrality_map,
      make_iterator_property_map(incoming.begin(), vertex_index),
      make_iterator_property_map(distance.begin(), vertex_index),
      make_iterator_property_map(dependency.begin(), vertex_index),
      make_iterator_property_map(path_count.begin(), vertex_index),
      vertex_index);
  }

  template<typename WeightMap>
  struct brandes_betweenness_centrality_heuristic_dispatch1
  {
    template<typename Graph,
             typename TrafficMatrix,
             typename CentralityMap,
             typename EdgeCentralityMap, typename VertexIndexMap>
    static void
    run(const Graph& g,
        TrafficMatrix traffic_matrix,
        CentralityMap centrality,
        EdgeCentralityMap edge_centrality_map, VertexIndexMap vertex_index,
        WeightMap weight_map)
    {
      brandes_betweenness_centrality_heuristic_dispatch2(g,
                                               traffic_matrix,
                                               centrality, edge_centrality_map,
                                               weight_map, vertex_index);
    }
  };

  template<>
  struct brandes_betweenness_centrality_heuristic_dispatch1<param_not_found>
  {
    template<typename Graph,
             typename TrafficMatrix,
             typename CentralityMap,
             typename EdgeCentralityMap, typename VertexIndexMap>
    static void
    run(const Graph& g,
        TrafficMatrix traffic_matrix,
        CentralityMap centrality,
        EdgeCentralityMap edge_centrality_map, VertexIndexMap vertex_index,
        param_not_found)
    {
      brandes_betweenness_centrality_heuristic_dispatch2(g,
                                               traffic_matrix,
                                               centrality, edge_centrality_map,
                                               vertex_index);
    }
  };

} } // end namespace detail::graph

template<typename Graph, typename TrafficMatrix, typename Param, typename Tag, typename Rest>
void
brandes_betweenness_centrality_heuristic(const Graph& g,
                               TrafficMatrix traffic_matrix,
                               const bgl_named_params<Param,Tag,Rest>& params
                               BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph,vertex_list_graph_tag))
{
  typedef bgl_named_params<Param,Tag,Rest> named_params;

  typedef typename get_param_type<edge_weight_t, named_params>::type ew;
  detail::graph::brandes_betweenness_centrality_heuristic_dispatch1<ew>::run(
    g,
    traffic_matrix,
    choose_param(get_param(params, vertex_centrality),
                 dummy_property_map()),
    choose_param(get_param(params, edge_centrality),
                 dummy_property_map()),
    choose_const_pmap(get_param(params, vertex_index), g, vertex_index),
    get_param(params, edge_weight));
}

} // end namespace boost

#endif //GRAPH_PARSER_CENTRALITY_H
