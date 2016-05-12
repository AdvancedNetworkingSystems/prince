/*
 * betweenness_centrality.cpp
 *
 *  Created on: 12 mag 2016
 *      Author: gabriel
 */

#include "betweenness_centrality.h"

using namespace std;

BetweennessCentrality::BetweennessCentrality(){

}

BetweennessCentrality::BetweennessCentrality(GraphManager gm): gm_(gm){
	init();
}

void BetweennessCentrality::init(GraphManager &gm){
	gm_ = gm;
	num_of_vertices_ = boost::num_vertices(gm_.g_);
}


void BetweennessCentrality::init(){
    num_of_vertices_ = boost::num_vertices(gm_.g_);

}
// BETWEENNESS CENTRALITY - NORMAL CALCULATION
void BetweennessCentrality::CalculateBetweennessCentrality(bool endpoints_inclusion) {
    initialize_betweenness_centrality();

    if (gm_.weighted_graph()) { // calculate BC for weighted graph
        //cout << "======= BCC - BC for weighted graph ======\n";
        typedef map<Edge, double> EdgeWeightStdMap;
        typedef boost::associative_property_map<EdgeIndexStdMap> EdgeWeightPMap;
        EdgeIndexStdMap edge_weight_std_map;
        EdgeWeightPMap edge_weight_pmap = EdgeWeightPMap(edge_weight_std_map);

        BGL_FORALL_EDGES(edge, gm_.g_, Graph) {
            edge_weight_std_map[edge] = gm_.g_[edge].cost;
        }
        boost::brandes_betweenness_centrality_endpoints_inclusion(gm_.g_,
            endpoints_inclusion,
            boost::centrality_map(
                v_centrality_pmap_).vertex_index_map(
                gm_.v_index_pmap()).weight_map(
                edge_weight_pmap)
        );
    }
    else { // for unweighted graph
        boost::brandes_betweenness_centrality_endpoints_inclusion(gm_.g_,
            endpoints_inclusion,
            boost::centrality_map(
                v_centrality_pmap_).vertex_index_map(
                gm_.v_index_pmap())
        );
    }

    boost::relative_betweenness_centrality(gm_.g_, v_centrality_pmap_);
}



void BetweennessCentrality::compose_bc_map(vector<pair<string, double> > &map){
 	  /* Write the heuristic and the normal version of betweenness centrality
 	      1st column: brandes_betweenness_centrality
 	      2nd column: heuristic_betweenness_centrality
 	   */

		vector<pair<string, double> > mapcopy;
        NameToIntMap nm = gm_.v_id_index_map();
     	for(auto id_index_pair: nm){
     		pair<string, double> item(id_index_pair.first, v_centrality_vec_.at(id_index_pair.second));
            mapcopy.push_back(item);
     	}
     	map=mapcopy;
}


// BETWEENNESS CENTRALITY
void BetweennessCentrality::initialize_betweenness_centrality() {
    v_centrality_vec_ = CentralityVec(num_of_vertices());
    v_centrality_pmap_ = CentralityPMap(v_centrality_vec_.begin(), gm_.v_index_pmap());
}

int const BetweennessCentrality::num_of_vertices() const {
    return num_of_vertices_;
}
