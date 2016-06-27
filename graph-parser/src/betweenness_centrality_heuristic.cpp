//
// Created by quynh on 1/9/16.
//

#include "betweenness_centrality_heuristic.h"
using namespace std;

/******************************
* Public functions
******************************/
BetweennessCentralityHeuristic::~BetweennessCentralityHeuristic(){
	//destroy it
}
BetweennessCentralityHeuristic::BetweennessCentralityHeuristic(){

}

BetweennessCentralityHeuristic::BetweennessCentralityHeuristic(GraphManager gm): gm_(gm){
	init();
}

void BetweennessCentralityHeuristic::init(GraphManager &gm){
	gm_=gm;
	init();

}


void BetweennessCentralityHeuristic::init() {
    // Set some variables
    num_of_vertices_ = boost::num_vertices(gm_.g_);
    component_vec_ = ComponentVec(boost::num_edges(gm_.g_), -1);
    component_map_ = ComponentMap(component_vec_.begin(), gm_.e_index_pmap());
}

/* GETTER */
int const BetweennessCentralityHeuristic::num_of_bcc() {
    return num_of_bcc_;
}

StringSet const& BetweennessCentralityHeuristic::all_art_points_id() const {
    return all_art_points_id_;
}

NameToDoubleMap const& BetweennessCentralityHeuristic::bc_score() const {
    return bc_score_;
}

NameToDoubleMap const& BetweennessCentralityHeuristic::bc_relative_score() const {
    return bc_relative_score_;
}


int const BetweennessCentralityHeuristic::num_of_vertices(){
    return num_of_vertices_;
}

// AUTO RUN
void BetweennessCentralityHeuristic::CalculateBetweennessCentrality() {
    /* Auto run all the necessary functions
    */
#ifdef LOG
	BOOST_LOG_TRIVIAL(info)<< "Running FindBiConnectedComponents" << endl;
#endif
    FindBiConnectedComponents();

    // Calculate Link Weight + Link Weight Reversed
#ifdef LOG
    BOOST_LOG_TRIVIAL(info)<< "Calculate Link Weight + Link Weight Reversed\n";
#endif
    CalculateLinkWeight();
    CalculateLinkWeightReversed();
    verify_link_weight();

    // Calculate Traffic Matrix
#ifdef LOG
    BOOST_LOG_TRIVIAL(info) << "Calculate Traffic Matrix\n";
#endif
    CalculateTrafficMatrix();

    // Calculate Betweenness Centrality Heuristic
#ifdef LOG
     BOOST_LOG_TRIVIAL(info) << "Calculate Betweenness Centrality Heuristic\n";
#endif
    CalculateBetweennessCentralityHeuristic();
}

// Search for the BiConnectedComponents  and populate BCC[]
void BetweennessCentralityHeuristic::FindBiConnectedComponents() {
    boost::biconnected_components(gm_.g_, component_map_,
                                  back_inserter(art_points_),
                                  boost::vertex_index_map(gm_.v_index_pmap()));

    // Set some necessary variables
    graphext::id_of_some_vertices(gm_.g_, art_points_, all_art_points_id_);
    reset_num_of_bcc();
#ifdef LOG
    BOOST_LOG_TRIVIAL(info) << "Total # of components: " << num_of_bcc_ << endl;
    BOOST_LOG_TRIVIAL(info) << "Create Sub Components\n";
#endif
    // Process the result from boost::biconnected_components
    CreateSubComponents();
}

// LINK WEIGHT
void BetweennessCentralityHeuristic::CalculateLinkWeight() {
    initialize_weight();
    initialize_queue();

    while (!Q.empty()) {
        QueueElem elem = Q.front();
        Q.pop();
        int comp_index = elem.component_index;
        string vertex_id = elem.vertex_id;

        if (elem.type == "component_vertex_pair") {
#ifdef LOG
        	BOOST_LOG_TRIVIAL(info) << "component_vertex_pair: " << comp_index << " - " << vertex_id << endl;
#endif
            process_component_vertex_pair(comp_index, vertex_id);
        }
        else if (elem.type == "vertex_component_pair") {
#ifdef LOG
        	BOOST_LOG_TRIVIAL(info) << "vertex_component_pair: " << comp_index << " - " << vertex_id << endl;
#endif
            process_vertex_component_pair(comp_index, vertex_id);
        }
    }
}

void BetweennessCentralityHeuristic::CalculateLinkWeightReversed() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        BCCs[i].calculate_weight_reversed(num_of_vertices_);
    }
}

// TRAFFIC MATRIX
void BetweennessCentralityHeuristic::CalculateTrafficMatrix() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        BCCs[i].CalculateTrafficMatrix();
    }
}

// BETWEENNESS CENTRALITY HEURISTIC
void BetweennessCentralityHeuristic::CalculateBetweennessCentralityHeuristic() {
#ifdef LOG
    BOOST_LOG_TRIVIAL(info) << "BETWEENNESS CENTRALITY HEURISTIC\n";
#endif
    //initialize_betweenness_centrality_heuristic();
    calculate_bc_inter();

    for (int i = 0; i < num_of_bcc_; ++i) {
        BCCs[i].CalculateBetweennessCentralityHeuristic();
    }

    double score;
    for (int i = 0; i < num_of_bcc_; ++i) {
        // For all points
        for (string id: BCCs[i].all_vertices_id()) {
            score = BCCs[i].get_betweenness_centrality(id);
            bc_score_[id] += score;
        }
    }

    // Update the BC score for articulation points
    for (string id : all_art_points_id_) {
        bc_score_[id] -= bc_inter_[id];
    }

    finalize_betweenness_centrality_heuristic();
#ifdef LOG
    BOOST_LOG_TRIVIAL(info) << "DONE WITH BETWEENNESS CENTRALITY\n";
#endif
}

//// HELPERS FOR OUTPUTTING RESULT
//void BetweennessCentralityHeuristic::print_all_sub_components() {
//    for (int i = 0; i < num_of_bcc_; ++i) {
//        cout << BCCs[i]; // Since I call another print() function inside, I cannot use cout
//        BCCs[i].print();
//    }
//}
//
//void BetweennessCentralityHeuristic::print_biconnected_components() {
//    cout << "\nArticulation points:\n";
//    outops::operator<<(cout, all_art_points_id_);
//
//    cout << "All Sub Components:\n\n";
//    for (int i = 0; i < num_of_bcc_; ++i) {
//        cout << "    " << i << endl;
//        outops::operator<<(cout, BCCs[i].all_vertices_id());
//    }
//}
//
//void BetweennessCentralityHeuristic::print_betweenness_centrality() {
//    BGL_FORALL_VERTICES(v, gm_.g_, Graph) {
//        double bc_score = boost::get(v_centrality_pmap_, v);
//        cout << gm_.g_[v].id << ": " << bc_score << endl;
//    }
//}
//
//void BetweennessCentralityHeuristic::print_betweenness_centrality_heuristic() {
//    outops::operator<< <double>(cout, bc_relative_score_);
//}
//
void BetweennessCentralityHeuristic::compose_bc_map(vector<pair<string, double> > &map){
    //HEURISTIC NOT WORKING WITH WEIGHTED LINKS

	vector<pair<string, double> > mapcopy(bc_relative_score_.begin(), bc_relative_score_.end());
    std::sort(mapcopy.begin(), mapcopy.end(), less_first<string, double>());
    map=mapcopy;

}


//void BetweennessCentralityHeuristic::write_all_betweenness_centrality(string filepath) {
//    /* Write the heuristic and the normal version of betweenness centrality
//    1st column: brandes_betweenness_centrality
//    2nd column: heuristic_betweenness_centrality
//    */
//
//    vector<pair<string, double> > mapcopy(bc_relative_score_.begin(), bc_relative_score_.end());
//    std::sort(mapcopy.begin(), mapcopy.end(), less_second<string, double>());
//
//    ofstream outFile(filepath.c_str());
//
//    Viter vi, ve;
//    size_t i = 0;
//    if (outFile.is_open()) {
//        for(auto id_bc_score_pair : mapcopy) {
//            string id = id_bc_score_pair.first;
//            double heuristic_bc_score = id_bc_score_pair.second;
//
//            int index = gm_.get_index_from_id(id);
//            double bc_score = v_centrality_vec_.at(index);
//
//            outFile << id << "\t" << setprecision(4) << fixed << bc_score << "\t" << heuristic_bc_score << endl;
//        }
//        // for (boost::tie(vi, ve) = boost::vertices(gm_.g_); vi != ve; ++vi) {
//        //     string id = gm_.g_[*vi].id;
//        //     outFile << id << "\t" << bc_relative_score_[id] << "\t" << v_centrality_vec_.at(i) << endl;
//        //     ++i;
//        // }
//    }
//    outFile.close();
//    cout << "Finish writing brandes and heurisic BC score to file " << filepath << endl;
//}
//
//void BetweennessCentralityHeuristic::print() {
//    cout << "\n\nBi-Connected Components\n\n";
//    cout << gm_;
//
//    print_biconnected_components();
//
//    cout << "\nHeuristic Betweenness Centrality Score:\n";
//    outops::operator<< <double>(cout, bc_score_);
//
//    cout << "\nHeuristic Betweenness Centrality Relative Score:\n";
//    outops::operator<< <double>(cout, bc_relative_score_);
//
//    cout << "\nNormal Betweenness Centrality Score:\n";
//    print_betweenness_centrality();
//}
//
//std::ostream& operator<<(std::ostream& os, const BetweennessCentralityHeuristic& rhs) {
//    cout << "\n\nBi-Connected Components\n\n";
//    cout << rhs.gm_;
//
//    cout << "\nArticulation points:\n";
//    outops::operator<<(cout, rhs.all_art_points_id());
//
//    cout << "\nHeuristic Betweenness Centrality Score:\n";
//    outops::operator<< <double>(cout, rhs.bc_score());
//
//    cout << "\nHeuristic Betweenness Centrality Relative Score:\n";
//    outops::operator<< <double>(cout, rhs.bc_relative_score());
//
//    return os;
//}

/******************************
* Private functions
******************************/
// SUB-COMPONENT
void BetweennessCentralityHeuristic::reset_num_of_bcc() {
    num_of_bcc_ = *std::max_element(component_vec_.begin(), component_vec_.end()) + 1;
}
//push the BCC in the list
void BetweennessCentralityHeuristic::CreateSubComponents() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        BCCs.push_back(SubComponent(gm_.weighted_graph()));
    }

    // Generating subgraph for each sub-component
    BGL_FORALL_EDGES(edge, gm_.g_, Graph) {
        Vertex source = boost::source(edge, gm_.g_);
        Vertex target = boost::target(edge, gm_.g_);

        int comp_index = boost::get(component_map_, edge);
        //cout << comp_index << " ";

        if (comp_index == -1) {
            cout << "ERROR: edge ";
            graphext::print_edge(gm_.g_, edge);
            cout << "not belonging to subcomponent\n";
        }
        else {
            Router r1 = gm_.g_[source];
            Router r2 = gm_.g_[target];
            Link l = gm_.g_[edge];
            if (r1 != r2) { // Do not add self-loop edge
                BCCs[comp_index].AddEdge(r1, r2, l);
            }
        }
    }

    // Finalizing each sub components
    for (int i = 0; i < num_of_bcc_; ++i) {
        BCCs[i].FinalizeSubComponent(all_art_points_id_);
    }
}

// LINK WEIGHT
void BetweennessCentralityHeuristic::initialize_weight() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        BCCs[i].initialize_weight();
    }
}
//push the BiConnectedComponents inside of the queue
void BetweennessCentralityHeuristic::initialize_queue() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        if (BCCs[i].art_points_id().size() == 1) {
            string vertex_id = *(BCCs[i].art_points_id().begin());
            string type = "component_vertex_pair";
            QueueElem elem = {
                .component_index = i,
                .vertex_id = vertex_id,
                .type = type,
            };
#ifdef LOG
            BOOST_LOG_TRIVIAL(info)  << "adding component_vertex_pair (" << i << " " << vertex_id << ")\n";
#endif
            Q.push(elem);
        }
    }
}

void BetweennessCentralityHeuristic::process_component_vertex_pair(int comp_index, string vertex_id) {
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

    int old_link_weight = BCCs[comp_index].get_weight_map(vertex_id);
    if (old_link_weight != -1) {
        if (link_weight != old_link_weight) {
#ifdef LOG
        	BOOST_LOG_TRIVIAL(fatal)  << "ERROR in Link Weight for comp " << comp_index << " | vertex " << vertex_id << old_link_weight << " | " << link_weight << endl;
#endif
        }
    }

    BCCs[comp_index].update_weight_map(vertex_id, link_weight);
#ifdef LOG
    BOOST_LOG_TRIVIAL(info)  << "  update weight for comp " << comp_index << " | vertex " << vertex_id << " = " << link_weight << endl;
#endif
    find_unknown_weight_wrt_art_point(vertex_id);
}

void BetweennessCentralityHeuristic::find_unknown_weight_wrt_art_point(string vertex_id) {
    int count = 0;
    int comp_index = -1;
#ifdef LOG
    BOOST_LOG_TRIVIAL(info)  << "find_unknown_weight_wrt_art_point " << vertex_id << "\n";
#endif

    for (int i = 0; i < num_of_bcc_; ++i) {
        if (BCCs[i].vertex_exists(vertex_id)) {
            if (BCCs[i].get_weight_map(vertex_id) == -1) {
#ifdef LOG
            	BOOST_LOG_TRIVIAL(info)  << "     comp_index = " << i << endl;
#endif
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

#ifdef LOG
        BOOST_LOG_TRIVIAL(info) << "        Add vertex_component_pair: " << comp_index << " - " << vertex_id << endl;
#endif
        Q.push(elem);
    }
}

void BetweennessCentralityHeuristic::process_vertex_component_pair(int comp_index, string vertex_id) {
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

    int old_link_weight = BCCs[comp_index].get_weight_map(vertex_id);
    if (old_link_weight != -1) {
        if (link_weight != old_link_weight) {
#ifdef LOG
            BOOST_LOG_TRIVIAL(info )<< "ERROR in Link Weight for comp " << comp_index << " | vertex " << vertex_id << old_link_weight << " | " << link_weight << endl;
#endif
        }
    }

    BCCs[comp_index].update_weight_map(vertex_id, link_weight);
#ifdef LOG
    BOOST_LOG_TRIVIAL(info) << "  update weight for vertex_component pair: " << comp_index << " | " << vertex_id << " = " << link_weight << endl;
#endif
    find_unknown_weight_wrt_component(comp_index);
}

void BetweennessCentralityHeuristic::find_unknown_weight_wrt_component(int comp_index) {
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
#ifdef LOG
        BOOST_LOG_TRIVIAL(info) << "Add component_vertex_pair: " << comp_index << " - " << vertex_id << endl;
#endif
        Q.push(elem);
    }
}

bool BetweennessCentralityHeuristic::verify_link_weight() {
    // Returns True if there is no negative value in Link Weight
    bool result = true;

    for (int i = 0; i < num_of_bcc_; ++i) {
        for (string id : BCCs[i].art_points_id()) {
            if (BCCs[i].get_weight_map(id) == -1) {
#ifdef LOG
            	BOOST_LOG_TRIVIAL(fatal) << "ERROR Link Weight for vertex " << id << " in component " << i << " = " << -1 << endl;
#endif

                result = false;
            }
        }
    }
}

// BETWEENNESS CENTRALITY - HEURISTIC
void BetweennessCentralityHeuristic::initialize_betweenness_centrality() {
    // Initialize bc_inter_ to be 0
    for (string id: all_art_points_id_) {
        bc_inter_[id] = 0;
    }

    StringSet all_vertices_id;
    graphext::id_of_all_vertices(gm_.g_, all_vertices_id);

    // Initialize bc_sum_, bc_score_ to be 0
    for (string id: all_vertices_id) {
        bc_sum_art_points_[id] = 0;
        bc_score_[id] = 0;
    }
}

void BetweennessCentralityHeuristic::calculate_bc_inter() {
    for (int i = 0; i < num_of_bcc_; ++i) {
        for (string id : BCCs[i].art_points_id()) {
            bc_inter_[id] += BCCs[i].get_weight_map(id) * BCCs[i].get_weight_reversed_map(id);
        }
    }

    if (!gm_.weighted_graph()) {
        for (string id : all_art_points_id_) {
            bc_inter_[id] /= 2;
        }
    }
}

void BetweennessCentralityHeuristic::finalize_betweenness_centrality_heuristic() {
    // Divide the bc_score_ by some factor
    int n = num_of_vertices();
    double factor = 2.0 / (n*n - 3*n + 2);
    NameToDoubleMap::iterator iter;
    for (iter = bc_score_.begin(); iter != bc_score_.end(); ++iter) {
        double old_value = iter->second;
        bc_relative_score_[iter->first] = old_value * factor;
    }
}

