#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_


/*INCLUDES */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../graph-parser_c/src/graph_parser.h"

/*DEFINES*/
#define true 1
#define false 0
#define LINE_SIZE 64


struct timers{
	double h_timer;
	double tc_timer;
	double exec_time;
};


typedef struct
routing_plugin_{
	char *recv_buffer;
	char *self_id;
	char *host;
	short port;
	int json_type;
	c_graph_parser *gp;

}routing_plugin;



#endif /* SRC_COMMON_H_ */
