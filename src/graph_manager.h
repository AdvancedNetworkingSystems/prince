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

    // CONSTRUCTOR
    GraphManager(); // constructor
    GraphManager(const GraphManager& other); // copy constructor
    GraphManager& operator=(GraphManager& rhs); // assignment operator
    // check out more here: http://www.cplusplus.com/articles/y8hv0pDG/

    // GETTERS
    const VertexIndexPMap& v_index_pmap() const;
    const EdgeIndexPMap& e_index_pmap() const;
    NameToIntMap const& v_id_index_map() const;

    // UPDATE GRAPH
    void AddEdge(VertexProperties vp1, VertexProperties vp2, EdgeProperties ep);
    void ResetVerticesAndEdgesIndexMap();

    // HELPERS
    bool vertex_existed(string s);
    const Vertex& get_vertex_from_id(string s);
    int get_index_from_id(string s);


    // OUTPUTTING THE RESULT
    // TODO: checkout utility.h, I can't overload << operator to output
    // the correct result. Therefore, I use print() ufnction as a replacement.
    void print();
    void print_v_index_pmap();
    void print_e_index_pmap();
    friend std::ostream& operator<<(std::ostream& os, const GraphManager& gm);

    // Variables
    Graph g_;
private:
    void reset_v_index_pmap();
    void reset_v_id_vertex_map();
    void reset_v_id_index_map(); // must be called after reset_v_index_pmap()

    void reset_e_index_pmap();
    void update_v_index_pmap(Vertex new_vertex);
    void update_e_index_pmap(Edge new_edge);

    // VARIABLES
    NameVertexMap v_id_vertex_map_;
    NameToIntMap v_id_index_map_;

    // VertexIndexPMap - since we use listS instead of vecS in Graph, this one maps each vertex to an id
    VertexIndexStdMap v_index_std_map_;
    VertexIndexPMap v_index_pmap_; //

    // EdgeIndexPMap - boost will return the component id that each edge in the graph belongs to.
    EdgeIndexStdMap e_index_std_map_;
    EdgeIndexPMap e_index_pmap_;
};

#endif //GRAPH_PARSER_GRAPH_MANAGER_H

