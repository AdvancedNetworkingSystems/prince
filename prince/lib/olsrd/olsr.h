#ifndef SRC_OLSR_H_
#define SRC_OLSR_H_

#include "topology_parser.h"
#include "common.h"
#include "socket.h"
/*inehrit methods from here */
#include "plugin_interface.h"

float parse_initial_timer(routing_plugin *o, char *cmd);

#define HELLO_TIMER_MESSAGE "/helloTimer\n"
#define TC_TIMER_MESSAGE "/tcTimer\n"
#define RESPONSE_SIZE (sizeof(char) * 24)

#endif /* SRC_OLSR_H_ */
