//
// Created by quynh on 1/9/16.
//

#ifndef GRAPH_PARSER_BI_CONNECTED_COMPONENTS_H
#define GRAPH_PARSER_BI_CONNECTED_COMPONENTS_H

#include <boost/graph/biconnected_components.hpp>
#include <queue>
#include "common.h"
#include "parser.h"
#include "utility.h"
#include "sub_component.h"


typedef std::vector<edges_size_type> ComponentVec;
typedef boost::iterator_property_map<ComponentVec::iterator, EdgeIndexMap> ComponentMap;

typedef map<Vertex, vector<int> > ArtPointComponentStdMap;
typedef boost::associative_property_map<ArtPointComponentStdMap> ArtPointComponentMap;

typedef std::map<int, vector<Vertex> > ComponentArtPointStdMap;
typedef boost::associative_property_map<ComponentArtPointStdMap> ComponentArtPointMap;

typedef vector<vector<Vertex> > Bcc_t;
typedef vector<vector<Vertex> >::iterator BccIter_t;



typedef struct {
    int component_index;
    Vertex art_point;
    string type;
} QueueElem;

class BiConnectedComponents {
private:
    // EdgeIndexMap - boost will return the component id that each edge in the graph belongs to.
    StdEdgeIndexMap e_index_std_map;
    EdgeIndexMap e_index_map;

    // VertexIndexMap - since we use listS instead of vecS, this one maps each vertex to an id
    StdVertexIndexMap v_index_std_map;

    int num_of_bcc = -1;

    std::queue<QueueElem> Q;

public:
    typedef vector<SubComponent> Component_t;
    typedef vector<SubComponent>::iterator ComponentIter_t;

    Graph g;

    VertexIndexMap v_index_map;
    ComponentVec component_vec;
    ComponentMap component_map;

    vector<Vertex> art_points;

    Bcc_t bcc_vertices;
    Component_t BCCs;

    ArtPointComponentStdMap art_point_component_std_map;
    ArtPointComponentMap art_point_component_map;

    ComponentArtPointStdMap component_art_point_std_map;
    ComponentArtPointMap component_art_point_map;


    BiConnectedComponents(const Graph &g);
    void init();
    void findBiConnectedComponents();
    void createSubComponents();
    void print();

    int num_bcc();
    void verticesInBCC();
    void testVerticesInBCC();

    bool hasVertex(Vertex v, int comp_index);
    void processArtPoints();
    void testProcessArtPoints();

    VertexVec get_art_points_for_component(int comp_index);
    vector<int> get_components_for_art_point(Vertex vertex);

    VertexVec get_normal_vertices_for_component(int comp_index);

    int num_of_vertices_in_component(int comp_index);

    // Calculate link weight (component tree weight)
    void compute_weight();
    void _initialize_weight();
    void _initialize_queue();
    void _find_unknown_weight_wrt_art_point(Vertex art_point);
    void _find_unknown_weight_wrt_component(int comp_index);
    void _find_unknown_weight_for_component(int comp_index, VertexVec& unknown_weight_vertices);
    bool _verify_weight(int comp_index, Vertex art_point, int weight);
    void print_weight();

    // Calculate Betweenness Centrality
    void findBetweennessCentrality();
    void printBetweennessCentrality();

    // Function used in debugging
    void _print_art_points();
    void _test_set_difference();

};


#endif //GRAPH_PARSER_BI_CONNECTED_COMPONENTS_H
