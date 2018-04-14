#ifndef SRC_OLSR_H_
#define SRC_OLSR_H_

#include "topology_parser.h"
#include "common.h"
#include "socket.h"
/*inehrit methods from here */
#include "plugin_interface.h"

float get_initial_timer(routing_plugin *o, char *cmd);

#endif /* SRC_OLSR_H_ */
