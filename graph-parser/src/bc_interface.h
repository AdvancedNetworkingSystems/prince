/*
 * bc_interface.h
 *
 *  Created on: 07 giu 2016
 *      Author: gabriel
 */

#ifndef GRAPH_PARSER_SRC_BC_INTERFACE_H_
#define GRAPH_PARSER_SRC_BC_INTERFACE_H_


#include "centrality.h"
#include "common.h"
#include "utility.h"
#include "graph_manager.h"

class BetweennessCentralityInterface{
public:
	virtual void init(GraphManager &gm)=0;
	virtual void init()=0;
	virtual void CalculateBetweennessCentrality()=0;
	virtual void compose_bc_map(vector<pair<string, double> > &map)=0;
	virtual void initialize_betweenness_centrality()=0;
	virtual int const num_of_vertices() =0;
	virtual ~BetweennessCentralityInterface(){};
};



#endif /* GRAPH_PARSER_SRC_BC_INTERFACE_H_ */
