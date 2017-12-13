#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

/*INCLUDES */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../graph-parser_c/src/graph_parser.h"

/*DEFINES*/
#define true 1
#define false 0
#define LINE_SIZE 64

struct timers {
        double h_timer;
        double tc_timer;
        double exec_time;
        double centrality;
};

typedef struct routing_plugin_ {
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

#endif /* SRC_COMMON_H_ */
