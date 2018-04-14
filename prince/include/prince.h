#ifndef SRC_PRINCE_H_
#define SRC_PRINCE_H_
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "config.h"
#include "config_proto.h"
#include "config_graph.h"
#include "load_plugin.h"
#include "prince_handler.h"
#include "topology_parser.h"


int compute_constants(prince_handler_t ph);
int compute_timers(prince_handler_t ph);
double get_self_bc(prince_handler_t ph);
void log_line(char *text, prince_handler_t ph);
void signal_callback_handler(int signum);
#endif /* SRC_PRINCE_H_ */
