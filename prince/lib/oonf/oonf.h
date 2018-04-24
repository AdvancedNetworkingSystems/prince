#ifndef SRC_OONF_H_
#define SRC_OONF_H_

#include "common.h"
#include "topology_parser.h"
#include "socket.h"
/*inehrit methods from here */
#include "plugin_interface.h"

#define HELLO_TIMER_MESSAGE "/config get interface.hello_interval/quit\n"
#define TC_TIMER_MESSAGE "/config get olsrv2.tc_interval/quit\n"
#define RESPONSE_SIZE (sizeof(char) * 64)


int push_timers(routing_plugin *o, struct timers t);
float parse_initial_timer(routing_plugin *o, const char *cmd);
#endif /* SRC_OONF_H_ */
