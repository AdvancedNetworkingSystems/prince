//
// Created by quynh on 1/9/16.
//

// Problem with deep copy
// http://stackoverflow.com/questions/6955578/subtraction-and-intersection-of-two-vectors-of-pointers-in-c

#ifndef GRAPH_PARSER_SUB_COMPONENT_H
#define GRAPH_PARSER_SUB_COMPONENT_H

#include <boost/graph/betweenness_centrality.hpp>
#include <boost/graph/copy.hpp>
#include "common.h"
#include "utility.h"

class SubComponent {
public:
    SubComponent();
    SubComponent(const StringSet art_points, const Graph sub_graph);

    // Getter & Setter
    StringSet art_points() const;
    void set_art_points(StringSet& art_points);

    // Manipulating subGraph
    int num_vertices();
    void AddEdge(Router r1, Router r2, Link l);
    bool vertex_existed(string s);
    const Vertex& get_vertex_from_id(string s);
    Graph const& sub_graph() const;

    // Output to console
    friend std::ostream& operator<<(std::ostream& os, const SubComponent& sc);

    void init();


    // calculate Link Weight
    void _initialize_weight();
    void setWeight(Vertex art_point, int value);
    int getWeight(Vertex art_point);
    int getReversedWeight(Vertex art_point);
    void printWeight();
    void _find_vertices_with_unknown_weight(VertexVec& unknown_weight_vertices);

    // Traffic Matrix
    int getTrafficMatrix(Vertex v_1, Vertex v_2);
    void _initializeTrafficMatrix();
    int _indexOfVertex(Vertex v);
    void _setTrafficMatrix(Vertex v_1, Vertex v_2, int value);
    void _computeTrafficMatrix();

    // Betweenness Centrality
    void _initializeBetweennessCentrality();
    void findBetweennessCentrality();
    void printBetweennessCentrality();

private:
    StringSet art_points_;
    StringVec normal_vertices; // vertices that are not articulation points
    // std::vector<Vertex*> normal_vertices;
    // TODO: should this one be public?
    std::map<std::string, Vertex> name_vertex_map_;
    Graph sub_graph_;
    VertexMap weightMap;
    VertexMap weightReversedMap;

    StdVertexIndexMap v_index_std_map;
    VertexIndexMap v_index_map;
    vector<vector<int> > trafficMatrix;

    CentralityVec v_centrality_vec;
    CentralityMap v_centrality_map;

    // Traffic Matrix

    // Betweenness Centrality
};


#endif //GRAPH_PARSER_SUB_COMPONENT_H
