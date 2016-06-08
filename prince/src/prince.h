/*
 * prince.h
 *
 *  Created on: 12 mag 2016
 *      Author: gabriel
 */

#ifndef SRC_PRINCE_H_
#define SRC_PRINCE_H_

#include <math.h>
#include <dlfcn.h>
#include "common.h"



struct constants{
	double O_H, O_TC, sq_lambda_H, sq_lambda_TC, R;
};


struct prince_handler{
	struct timers def_t, opt_t;
	struct constants c;
	char *self_id, *host;
	c_graph_parser *gp;
	map_id_bc_pair *bc_map;
	map_id_degree_pair *degree_map;
	routing_plugin *rp;
	int proto, heuristic, weights;
};


int main(int argc, char *argv[]);

struct prince_handler* new_prince_handler();
int compute_constants(struct prince_handler *ph);
int compute_timers(struct prince_handler *ph);
void delete_prince_handler(struct prince_handler*);
int read_config_file(struct prince_handler *ph, char *filepath);


#endif /* SRC_PRINCE_H_ */

