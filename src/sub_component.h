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
#include "graph_manager.h"

class SubComponent {
public:
    SubComponent();

    // Getter & Setter
    GraphManager const& gm() const;
    StringSet const& all_vertices_id() const;
    StringSet const& art_points_id() const;
    StringSet const& non_art_points_id() const;

    NameToIntMap const& weight_map() const;
    NameToIntMap const& weight_reversed_map() const;

    vector< vector< int > > const& traffic_matrix() const;

    // CREATE SUB-COMPONENT
    void AddEdge(Router r1, Router r2, Link l);
    void FinalizeSubComponent(StringSet all_art_points_id);

    // LINK WEIGHT CALCULATION
    void initialize_weight();
    int get_weight_map(string name);
    int get_weight_reversed_map(string name);
    void update_weight_map(string name, int value);

    // TRAFFIC MATRIX CALCULATION
    void CalculateTrafficMatrix();
    void initialize_traffic_matrix();
    int get_traffic_matrix(string name_1, string name_2);
    void update_traffic_matrix(string name_1, string name_2, int value);

    // HELPERS
    int num_of_vertices();
    int index_of_vertex_id(string vertex_id);
    bool vertex_exists(string name);
    string first_vertex_id_with_unknown_weight();

    // HELPERS FOR OUTPUTTING RESULT
    void print_traffic_matrix();
    friend std::ostream& operator<<(std::ostream& os, const SubComponent& sc);


    // OLD CODE
    void printWeight();
    void _find_vertices_with_unknown_weight(VertexVec& unknown_weight_vertices);

    // Traffic Matrix
    void _computeTrafficMatrix();

    // Betweenness Centrality
    void _initializeBetweennessCentrality();
    void findBetweennessCentrality();
    void printBetweennessCentrality();

    // NEW CODE
    GraphManager gm_;

private:
    StringSet all_vertices_id_;
    StringSet art_points_id_;
    StringSet non_art_points_id_; // vertices that are not articulation points

    NameToIntMap weight_map_;
    NameToIntMap weight_reversed_map_;

    NameToIntMap name_index_map_;
    vector<vector<int> > traffic_matrix_;

    // Traffic Matrix

    // Betweenness Centrality
};


#endif //GRAPH_PARSER_SUB_COMPONENT_H
