#ifndef SRC_PRINCE_HANDLER_H_
#define SRC_PRINCE_HANDLER_H_

typedef struct prince_handler * prince_handler_t;

#include "common.h"
#include "config.h"
#include "load_plugin.h"

struct constants {
	double O_H, O_TC, sq_lambda_H, sq_lambda_TC, R;
};

#define INVALID_PRINCE_HANDLER NULL
#define PRINCE_HANDLER_SIZE sizeof(struct prince_handler)


struct prince_handler {
	bool             heuristic, weights, recursive, stop_unchanged, multithreaded;
	c_graph_parser   *gp;
	char             *command, *proto, *log_file;
	char             *self_id, *host;
	int              port, refresh, json_type, sleep_onfail, timer_port;
	map_id_degree_bc *bc_degree_map;
	routing_plugin   *rp;
	struct           timers def_t, opt_t;
	struct constants c;
	void             *plugin_handle;
        proto_config_t   config;
};

prince_handler_t new_prince_handler(char * conf_file);
void             free_prince_handler(prince_handler_t ph);

#endif /*SRC_PRINCE_HANDLER_H_*/
