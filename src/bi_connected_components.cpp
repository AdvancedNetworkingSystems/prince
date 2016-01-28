//
// Created by quynh on 1/9/16.
//

#include "bi_connected_components.h"
using namespace std;

/******************************
* Public functions
******************************/
BiConnectedComponents::BiConnectedComponents(GraphManager &gm) : gm_(gm) {
    cout << "BCC constructor \n";
    init();
    FindBiConnectedComponents();
}

void BiConnectedComponents::init() {
    // Set some variables
    num_of_vertices_ = boost::num_vertices(gm_.g_);

    component_vec_ = ComponentVec(boost::num_edges(gm_.g_), 0);
    component_map_ = ComponentMap(component_vec_.begin(), gm_.e_index_map());
}

/* GETTER */
StringSet const& BiConnectedComponents::all_art_points_id() const {
    return all_art_points_id_;
}

/* SUB-COMPONENT */
void BiConnectedComponents::FindBiConnectedComponents() {
    cout << "Running FindBiConnectedComponents" << endl;

    boost::biconnected_components(gm_.g_, component_map_,
                                  back_inserter(art_points_),
                                  boost::vertex_index_map(gm_.v_index_map()));

    // Set some variables
    graphext::id_of_some_vertices(gm_.g_, art_points_, all_art_points_id_);

    // Process the result from boost::biconnected_components
    BCCs = Component_t(num_of_bcc());
    CreateSubComponents();

    // Calculate Link Weight
    cout << "Calculate Link Weight\n";
    CalculateLinkWeight();

    // Calculate Traffic Matrix
    cout << "Calculate Traffic Matrix\n";
    CalculateTrafficMatrix();

    // Calculate Betweenness Centrality
    cout << "Calculate Betweenness Centrality\n";
    CalculateBetweennessCentrality();

    print();
}

void BiConnectedComponents::CreateSubComponents() {
    // Generating subgraph for each sub-component
    BGL_FORALL_EDGES(edge, gm_.g_, Graph) {
        int comp_index = boost::get(component_map_, edge);
        Router r1 = gm_.g_[boost::source(edge, gm_.g_)];
        Router r2 = gm_.g_[boost::target(edge, gm_.g_)];
        Link l = gm_.g_[edge];
        BCCs[comp_index].AddEdge(r1, r2, l);
    }

    // Finalizing each sub components
    for (int i = 0; i < num_of_bcc(); ++i) {
        BCCs[i].FinalizeSubComponent(all_art_points_id_);
    }
}

/* LINK WEIGHT */
void BiConnectedComponents::CalculateLinkWeight() {
    initialize_weight();
    initialize_queue();

    while (!Q.empty()) {
        QueueElem elem = Q.front();
        Q.pop();
        int comp_index = elem.component_index;
        string vertex_id = elem.vertex_id;

        if (elem.type == "component_vertex_pair") {
            // cout << "component_vertex_pair: " << comp_index << " - " << vertex_id << endl;
            process_component_vertex_pair(comp_index, vertex_id);
        }
        else if (elem.type == "vertex_component_pair") {
            // cout << "vertex_component_pair: " << comp_index << " - " << vertex_id << endl;
            process_vertex_component_pair(comp_index, vertex_id);
        }
    }
}

/* TRAFFIC MATRIX */
void BiConnectedComponents::CalculateTrafficMatrix() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        BCCs[i].CalculateTrafficMatrix();
    }
}

/* BETWEENNESS CENTRALITY */
void BiConnectedComponents::CalculateBetweennessCentrality() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        BCCs[i].CalculateBetweennessCentrality();
    }
}

/* HELPERS */
int BiConnectedComponents::num_of_bcc() {
    if (num_of_bcc_ == -1) { // calculate it
        // +1 to counteract for the zero-indexing
        num_of_bcc_ = *std::max_element(component_vec_.begin(), component_vec_.end()) + 1;
    }

    return num_of_bcc_;
}

/* HELPERS FOR OUTPUTTING RESULT */
void BiConnectedComponents::print() {
    for (int i = 0; i < num_of_bcc(); ++i) {
        cout << BCCs[i] << endl;
    }
}

std::ostream& operator<<(std::ostream& os, const BiConnectedComponents& rhs) {
    cout << "\n\nBi-Connected Components\n\n";
    cout << rhs.gm_;

    cout << "\nArticulation points:\n";
    outops::operator<<(cout, rhs.all_art_points_id());
    return os;

    // TODO: output the result of BCC
    // Test the biconnected components
    // Eiter ei, ei_end;
    // size_t i = 0;
    // for (boost::tie(ei, ei_end) = boost::edges(gm_.g_); ei != ei_end; ++ei) {
    //     string source = gm_.g_[(*ei).m_source].id;
    //     string target = gm_.g_[(*ei).m_target].id;
    //     edges_size_type comp = component_vec_.at(i);
    //     cout << source << " " << target << " " << comp << endl;
    //     ++i;
    // }
}

/******************************
* Private functions
******************************/

/* LINK WEIGHT */
void BiConnectedComponents::initialize_weight() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        BCCs[i].initialize_weight();
    }
}

void BiConnectedComponents::initialize_queue() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        if (BCCs[i].art_points_id().size() == 1) {
            string vertex_id = *(BCCs[i].art_points_id().begin());
            string type = "component_vertex_pair";
            QueueElem elem = {
                .component_index = i,
                .vertex_id = vertex_id,
                .type = type,
            };

            Q.push(elem);
        }
    }
}

void BiConnectedComponents::process_component_vertex_pair(int comp_index, string vertex_id) {
    int size = BCCs[comp_index].num_of_vertices() - 1;
    for (string s : BCCs[comp_index].art_points_id()) {
        if (s.compare(vertex_id) != 0) {
            int weight = BCCs[comp_index].get_weight_map(s);
            if (weight != -1) {
                size += num_of_vertices_ - weight - 1;
            }
        }
    }
    int link_weight = size;
    BCCs[comp_index].update_weight_map(vertex_id, link_weight);
    find_unknown_weight_wrt_art_point(vertex_id);
}

void BiConnectedComponents::find_unknown_weight_wrt_art_point(string vertex_id) {
    int count = 0;
    int comp_index = -1;
    for (int i = 0; i < num_of_bcc_; ++i) {
        if (BCCs[i].vertex_exists(vertex_id)) {
            if (BCCs[i].get_weight_map(vertex_id) == -1) {
                ++count;
                comp_index = i;
            }

            if (count > 1) break;
        }
    }

    if (count == 1) {
        // Add new element to QueueElem
        QueueElem elem = {
            .component_index = comp_index,
            .vertex_id = vertex_id,
            .type = "vertex_component_pair"
        };

        // cout << "Add vertex_component_pair: " << comp_index << " - " << vertex_id << endl;
        Q.push(elem);
    }
}

void BiConnectedComponents::process_vertex_component_pair(int comp_index, string vertex_id) {
    int size = 0;

    for (int i = 0; i < num_of_bcc_; ++i) {
        if (i != comp_index) {
            if (BCCs[i].vertex_exists(vertex_id)) {
                int weight = BCCs[i].get_weight_map(vertex_id);
                if (weight != -1) {
                    size += weight;
                }
            }
        }
    }

    int link_weight = num_of_vertices_ - 1 - size;
    BCCs[comp_index].update_weight_map(vertex_id, link_weight);
    find_unknown_weight_wrt_component(comp_index);
}

void BiConnectedComponents::find_unknown_weight_wrt_component(int comp_index) {
    // The 'counting' solution is found here: http://stackoverflow.com/questions/5517615/counting-number-of-same-values-in-map
    typedef NameToIntMap::value_type MapEntry;
    int count = std::count_if(
        BCCs[comp_index].weight_map().begin(),
        BCCs[comp_index].weight_map().end(),
        second_equal_to<MapEntry>(-1));

    if (count == 1) {
        // Find the vertex id for the vertex with unknown link weight
        string vertex_id = BCCs[comp_index].first_vertex_id_with_unknown_weight();

        QueueElem elem = {
            .component_index = comp_index,
            .vertex_id = vertex_id,
            .type = "component_vertex_pair",
        };
        // cout << "Add component_vertex_pair: " << comp_index << " - " << vertex_id << endl;
        Q.push(elem);
    }
}