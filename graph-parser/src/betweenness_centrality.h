/*
 * betweenness_centrality.h
 *
 *  Created on: 12 mag 2016
 *      Author: gabriel
 */

#ifndef GRAPH_PARSER_BETWEENNESS_CENTRALITY_H
#define GRAPH_PARSER_BETWEENNESS_CENTRALITY_H

#include <boost/graph/biconnected_components.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include "centrality.h"
#include "common.h"
#include "utility.h"
#include "graph_manager.h"

class BetweennessCentrality{
public:
	BetweennessCentrality();
	BetweennessCentrality(GraphManager gm);
	void init(GraphManager &gm);
	void init();
	void CalculateBetweennessCentrality(bool endpoints_inclusion = true);
	void compose_bc_map(vector<pair<string, double> > &map);
	void initialize_betweenness_centrality();
	int const num_of_vertices() const;

protected:
    GraphManager gm_;
    CentralityVec v_centrality_vec_;
    CentralityPMap v_centrality_pmap_;
    int num_of_vertices_ = -1;
};

#endif
