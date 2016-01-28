//
// Created by quynh on 1/9/16.
//

#ifndef GRAPH_PARSER_BI_CONNECTED_COMPONENTS_H
#define GRAPH_PARSER_BI_CONNECTED_COMPONENTS_H

#include <boost/graph/biconnected_components.hpp>
#include <queue>
#include "common.h"
#include "parser.h" // ? check to remove
#include "utility.h"
#include "sub_component.h"
#include "graph_manager.h"


typedef std::vector<edges_size_type> ComponentVec;
typedef boost::iterator_property_map<ComponentVec::iterator, EdgeIndexMap> ComponentMap;

typedef map<string, vector<int> > VertexIdToComponentStdMap;
typedef boost::associative_property_map<VertexIdToComponentStdMap> VertexIdToComponentMap;

typedef std::map<int, vector<string> > ComponentToVertexIdStdMap;
typedef boost::associative_property_map<ComponentToVertexIdStdMap> ComponentToVertexIdMap;

typedef vector<vector<Vertex> > Bcc_t;
typedef vector<vector<Vertex> >::iterator BccIter_t;

typedef struct {
    int component_index;
    string vertex_id;
    string type;
} QueueElem;

class BiConnectedComponents {
public:
    BiConnectedComponents(GraphManager &gm);
    void init();

    // Getter functions
    set<string> const& all_art_points_id() const;
    int num_of_vertices() const;

    // SUB-COMPONENT
    void FindBiConnectedComponents();
    void CreateSubComponents();

    // LINK WEIGHT - calculation for all sub-components
    void CalculateLinkWeight();

    // TRAFFIC MATRIX - calculation for all sub-components
    void CalculateTrafficMatrix();

    // BETWEENNESS CENTRALITY
    void CalculateBetweennessCentrality();

    // HELPERS
    int num_of_bcc();

    // HELPERS FOR OUTPUTTING RESULT
    void print();
    friend std::ostream& operator<<(std::ostream& os, const BiConnectedComponents& rhs);

    // Public variables
    GraphManager gm_;
    typedef vector<SubComponent> Component_t;
    typedef vector<SubComponent>::iterator ComponentIter_t;
    Component_t BCCs;

private:
    // LINK WEIGHT - calculation for all sub-components
    void initialize_weight();
    void initialize_queue();
    void process_component_vertex_pair(int comp_index, string vertex_id);
    void process_vertex_component_pair(int comp_index, string vertex_id);
    void find_unknown_weight_wrt_art_point(string vertex_id);
    void find_unknown_weight_wrt_component(int comp_index);

    // Private variables
    ComponentVec component_vec_;
    ComponentMap component_map_;
    vector<Vertex> art_points_;

    int num_of_bcc_ = -1;
    int num_of_vertices_ = -1;

    StringSet all_art_points_id_;

    std::queue<QueueElem> Q;

    // Old code
    void processArtPoints();
    void testProcessArtPoints();

    VertexVec get_normal_vertices_for_component(int comp_index);

    // Calculate Betweenness Centrality
    void findBetweennessCentrality();
    void printBetweennessCentrality();

    // Function used in debugging
    void _print_art_points();
    void _test_set_difference();

};


#endif //GRAPH_PARSER_BI_CONNECTED_COMPONENTS_H
