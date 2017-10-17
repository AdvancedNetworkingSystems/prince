#ifndef SRC_PRINCE_H_
#define SRC_PRINCE_H_
#include <math.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "common.h"
#include "parser.h"
#include "config.h"

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
	bool heuristic, weights, recursive, stop_unchanged, multithreaded, degree;
	void *plugin_handle;
};


int main(int argc, char *argv[]);

struct prince_handler* new_prince_handler(char * conf_file);
void delete_prince_handler(struct prince_handler*);
int compute_constants(struct prince_handler *ph);
int compute_timers(struct prince_handler *ph);
int read_config_file(struct prince_handler *ph, char *filepath);
double get_self_bc(struct prince_handler *ph);
void log_line(char *text, struct prince_handler* ph);
#endif /* SRC_PRINCE_H_ */
