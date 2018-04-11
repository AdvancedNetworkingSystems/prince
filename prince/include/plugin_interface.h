#ifndef SRC_PLUGIN_INTERFACE_H_
#define SRC_PLUGIN_INTERFACE_H_

#include "common.h"


routing_plugin*  new_plugin(char* host, int port, int json_type, int timer_port);
int              get_topology(routing_plugin *o);
int              get_initial_timers(routing_plugin *o, struct timers *t);
int              push_timers(routing_plugin *o, struct timers t);
void             delete_plugin(routing_plugin* o);

#endif
