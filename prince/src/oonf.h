/*
 * olsr.h
 *
 *  Created on: 25 mag 2016
 *      Author: gabriel
 */

#ifndef SRC_OONF_H_
#define SRC_OONF_H_

#include "common.h"

#define LINE_SIZE 64
#define BUFFER_SIZE 1024


typedef struct
oonf_routing_plugin_{
	char *recv_buffer;
	char *host;
	short port;
	c_graph_parser *gp;
}oonf_routing_plugin;


//PUBLIC FUNCTIONS
oonf_routing_plugin* new_oonf_plugin(char* host, c_graph_parser *gp);
int get_netjson_topology(oonf_routing_plugin *o);
int oonf_push_timers(oonf_routing_plugin *o, struct timers t);
void oonf_delete_oonf_plugin(oonf_routing_plugin* o);


//PRIVATE FUNCTIONS
int	_send_telnet_cmd(int sd, char* cmd);

#endif /* SRC_OONF_H_ */
