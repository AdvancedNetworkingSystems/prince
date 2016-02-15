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


typedef std::vector<int> ComponentVec; // use int instead of edges_size_type because I want to set default value to be -1
typedef boost::iterator_property_map<ComponentVec::iterator, EdgeIndexPMap> ComponentMap;

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
    BiConnectedComponents(GraphManager &gm, bool weighted_graph = true);
    void init();

    // Getter functions
    int const num_of_bcc();
    int const num_of_vertices() const;
    StringSet const& all_art_points_id() const;
    NameToDoubleMap const& bc_score() const;
    NameToDoubleMap const& bc_relative_score() const;
    bool weighted_graph() const;

    // Setter functions
    void set_weighted_graph(bool rhs);

    // Auto run
    void run();

    // SUB-COMPONENT
    void FindBiConnectedComponents();

    // LINK WEIGHT - calculation for all sub-components
    void CalculateLinkWeight();
    void CalculateLinkWeightReversed();

    // TRAFFIC MATRIX - calculation for all sub-components
    void CalculateTrafficMatrix();

    // BETWEENNESS CENTRALITY HEURISTIC
    void CalculateBetweennessCentralityHeuristic();

    // BETWEENNESS CENTRALITY
    void CalculateBetweennessCentrality();

    // HELPERS FOR OUTPUTTING RESULT
    void print_all_sub_components();

    void print_betweenness_centrality();
    void print_betweenness_centrality_heuristic();

    void write_all_betweenness_centrality(string filepath);

    void print();
    friend std::ostream& operator<<(std::ostream& os, const BiConnectedComponents& rhs);

    // Public variables
    GraphManager gm_;
    bool weighted_graph_;
    typedef vector<SubComponent> Component_t;
    typedef vector<SubComponent>::iterator ComponentIter_t;
    Component_t BCCs;

private:
    // SUB-COMPONENT
    void reset_num_of_bcc();
    void CreateSubComponents();

    // LINK WEIGHT - calculation for all sub-components
    void initialize_weight();
    void initialize_queue();
    void process_component_vertex_pair(int comp_index, string vertex_id);
    void process_vertex_component_pair(int comp_index, string vertex_id);
    void find_unknown_weight_wrt_art_point(string vertex_id);
    void find_unknown_weight_wrt_component(int comp_index);
    bool verify_link_weight();

    // BETWEENNESS CENTRALITY HEURISTIC
    void initialize_betweenness_centrality_heuristic();
    void calculate_bc_inter();
    void finalize_betweenness_centrality_heuristic();

    // BETWEENNESS CENTRALITY
    void initialize_betweenness_centrality();

    // Private variables
    ComponentVec component_vec_;
    ComponentMap component_map_;
    vector<Vertex> art_points_;
    StringSet all_art_points_id_;

    int num_of_bcc_ = -1;
    int num_of_vertices_ = -1;

    std::queue<QueueElem> Q;

    // bc_score_ will be updated gradually, first with the score for non art points, and then the score for art points
    // Betweenness Centrality Heuristic
    NameToDoubleMap bc_score_;
    NameToDoubleMap bc_relative_score_;
    NameToDoubleMap bc_sum_art_points_; // summing all the bc score for articulation points
    NameToDoubleMap bc_inter_;

    // Betweenness Centrality - standard calculation
    CentralityVec v_centrality_vec_;
    CentralityPMap v_centrality_pmap_;
};

#endif //GRAPH_PARSER_BI_CONNECTED_COMPONENTS_H
