#ifndef SRC_OLSR_H_
#define SRC_OLSR_H_

#include "../../src/parser.h"
#include "../../src/common.h"
#include "../../src/socket.h"
/*inehrit methods from here */
#include "../../src/plugin_interface.h"

float get_initial_timer(routing_plugin* o, char* cmd);

#endif /* SRC_OLSR_H_ */
