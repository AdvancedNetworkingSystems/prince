/*
 * oonf.h
 *
 *  Created on: 25 mag 2016
 *      Author: gabriel
 */

#ifndef SRC_OLSR_H_
#define SRC_OLSR_H_

#include <strings.h>
#include <stdio.h>

#include "common.h"

#define LINE_SIZE 64
#define BUFFER_SIZE 1024


typedef struct
olsr_routing_plugin_{
	char *recv_buffer;
	char *host;
	short port;
	c_graph_parser *gp;
	map_id_bc_pair *bc_map;
	map_id_degree_pair *degree_map;

}olsr_routing_plugin;


//PUBLIC FUNCTIONS
olsr_routing_plugin* new_olsr_plugin(char* host, c_graph_parser *gp);
int get_jsoninfo_topology(olsr_routing_plugin *o);
int olsr_push_timers(olsr_routing_plugin *o, struct timers t);
void delete_olsr_plugin(olsr_routing_plugin* o);


//PRIVATE FUNCTIONS


#endif /* SRC_OLSR_H_ */
