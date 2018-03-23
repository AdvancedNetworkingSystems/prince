#ifndef SRC_OONF_H_
#define SRC_OONF_H_

#include "common.h"
#include "topology_parser.h"
#include "socket.h"
/*inehrit methods from here */
#include "plugin_interface.h"

routing_plugin* new_plugin(char* host, int port, c_graph_parser *gp, int json_type, int timer_port);
int             get_topology(routing_plugin *o);
int             push_timers(routing_plugin *o, struct timers t);
void            delete_plugin(routing_plugin* o);
int             get_initial_timers(routing_plugin *o, struct timers *t);
float           parse_initial_timer(routing_plugin *o, const char *cmd);
#endif /* SRC_OONF_H_ */
