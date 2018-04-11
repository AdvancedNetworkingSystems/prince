#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#include "graph_parser.h"

#define true 1
#define false 0
#define LINE_SIZE 64


struct timers{
	double h_timer;
	double tc_timer;
	double exec_time;
	double centrality;
};


typedef struct
routing_plugin_{
	char *recv_buffer;
	char *self_id;
	char *host;
	struct topology *t;
	short port;
	short timer_port;
	int json_type;
	int sd;
	c_graph_parser *gp;

} routing_plugin;

routing_plugin* (*new_plugin_p)(char* host, int port, int json_type, int timer_port);

int  (*get_initial_timers_p) (routing_plugin *o, struct timers *t);
int  (*get_topology_p)       (routing_plugin *o);
int  (*push_timers_p)        (routing_plugin *o, struct timers t);
void (*delete_plugin_p)      (routing_plugin *o);

#endif /* SRC_COMMON_H_ */
