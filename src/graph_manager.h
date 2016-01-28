//
// Created by quynh on 1/15/15.
//

#ifndef GRAPH_PARSER_GRAPH_MANAGER_H
#define GRAPH_PARSER_GRAPH_MANAGER_H

#include "common.h"
#include "utility.h"

class GraphManager {
public:
    typedef boost::vertex_bundle_type<Graph>::type VertexProperties; // aka Router in common.h
    typedef boost::edge_bundle_type<Graph>::type EdgeProperties; // aka Link in common.h
    typedef std::map<std::string, Vertex> NameVertexMap;

    GraphManager(); // constructor
    GraphManager(const GraphManager& other); // copy constructor
    GraphManager& operator=(GraphManager& rhs); // assignment operator
    // check out more here: http://www.cplusplus.com/articles/y8hv0pDG/

    void AddEdge(VertexProperties vp1, VertexProperties vp2, EdgeProperties ep);
    bool vertex_existed(string s);
    const Vertex& get_vertex_from_id(string s);

    StringSet vertices_id();

    void ResetVerticesAndEdgesIndexMap();

    // Function to print results
    // TODO: checkout utility.h, I can't overload << operator to output
    // the correct result. Therefore, I use print() ufnction as a replacement.
    void print_v_index_map();
    void print_e_index_map();

    // Getter
    const VertexIndexMap& v_index_map() const;
    const EdgeIndexMap& e_index_map() const;

    // Output Operators
    friend std::ostream& operator<<(std::ostream& os, const GraphManager& gm);

    Graph g_;
private:
    void reset_v_id_vertex_map();
    void reset_v_index_map();
    void reset_e_index_map();
    void update_v_index_map(Vertex new_vertex);
    void update_e_index_map(Edge new_edge);

    NameVertexMap v_id_vertex_map_;

    // VertexIndexMap - since we use listS instead of vecS, this one maps each vertex to an id
    VertexIndexStdMap v_index_std_map_;
    VertexIndexMap v_index_map_; //

    // EdgeIndexMap - boost will return the component id that each edge in the graph belongs to.
    EdgeIndexStdMap e_index_std_map_;
    EdgeIndexMap e_index_map_;
};

#endif //GRAPH_PARSER_GRAPH_MANAGER_H

