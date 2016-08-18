/*
 * plugin_interface.h
 *
 *  Created on: 02 giu 2016
 *      Author: gabriel
 */
#ifndef SRC_PLUGIN_INTERFACE_H_
#define SRC_PLUGIN_INTERFACE_H_

#include "common.h"


routing_plugin* new_plugin(char* host, int port, c_graph_parser *gp, int json_type);
int get_topology(routing_plugin *o);
int push_timers(routing_plugin *o, struct timers t);
void delete_plugin(routing_plugin* o);


#endif
