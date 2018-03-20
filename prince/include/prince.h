#ifndef SRC_PRINCE_H_
#define SRC_PRINCE_H_
#include "common.h"
#include "config.h"
#include "parser.h"
#include <dlfcn.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

typedef struct prince_handler * prince_handler_t;

struct constants{
	double O_H, O_TC, sq_lambda_H, sq_lambda_TC, R;
};


struct prince_handler{
	struct timers def_t, opt_t;
	struct constants c;
	char *self_id, *host;
	char *command, *proto, *log_file;
	c_graph_parser *gp;
	map_id_degree_bc *bc_degree_map;
	routing_plugin *rp;
	int port,refresh,json_type, sleep_onfail, timer_port;
	bool heuristic, weights, recursive, stop_unchanged, multithreaded;
	void *plugin_handle;
};


int main(int argc, char *argv[]);

prince_handler_t new_prince_handler(char * conf_file);
void delete_prince_handler(prince_handler_t ph);
int  compute_constants(prince_handler_t ph);
int  compute_timers(prince_handler_t ph);
int  read_config_file(prince_handler_t ph, char *filepath);
double get_self_bc(prince_handler_t ph);
void log_line(char *text, prince_handler_t ph);
void signal_callback_handler(int signum);
#endif /* SRC_PRINCE_H_ */
