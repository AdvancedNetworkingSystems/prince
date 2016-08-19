/*
Copyright (c) 2016 Gabriele Gemmi <gabriel@autistici.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/*
 * prince.c
 *
 *  Created on: 12 mag 2016
 *      Author: gabriel
 */
#include "prince.h"
#include "time.h"

routing_plugin* (*new_plugin)(char* host, int port, c_graph_parser *gp, int json_type);
int (*get_topology)(routing_plugin *o);
int (*push_timers)(routing_plugin *o, struct timers t);
void (*delete_plugin)(routing_plugin* o);

/**
 * Main routine of Prince. Collect topology, parse it, calculate bc and timers, push them back.
 * @param argv[1] <- config filename
 * @return 1 on success, 0 on error
 */
int
main(int argc, char* argv[]){
	struct prince_handler *ph= new_prince_handler(argv[1]);


	/*cycle each 'refresh' seconds*/
	do{
		sleep(ph->refresh);
		ph->gp = new_graph_parser(ph->weights, ph->heuristic);
		ph->rp = new_plugin(ph->host, ph->port, ph->gp, ph->json_type);
		if(!get_topology(ph->rp)){
			printf("Error getting topology");
			continue;

		}
		if(ph->rp->self_id)
			ph->self_id = strdup(ph->rp->self_id);
		clock_t start = clock();
		graph_parser_calculate_bc(ph->gp);
		clock_t end = clock();
		graph_parser_compose_degree_bc_map(ph->gp, ph->bc_degree_map);
		ph->opt_t.exec_time = (double)(end - start) / CLOCKS_PER_SEC;
		printf("Calculation time: %fs\n", ph->opt_t.exec_time);
		if (!compute_timers(ph)){
			delete_prince_handler(ph);
			continue;
		}
		printf("\nId of the node we are computing is: %s\n", ph->self_id);
		if (!push_timers(ph->rp, ph->opt_t)){
			delete_prince_handler(ph);
			continue;
		}
		delete_plugin(ph->rp);
	}while(ph->refresh);
	delete_prince_handler(ph);
	return 1;

}

/**
 * Initalize a new prince handler
 * @param host host address as a string
 * @return pointer to prince handler
 */
struct prince_handler*
new_prince_handler(char * conf_file){
	struct prince_handler* ph = (struct prince_handler*) malloc(sizeof(struct prince_handler));
	ph->def_t.h_timer=2.0;
	ph->def_t.tc_timer=5.0;
	ph->bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));
	ph->weights=0;
	ph->heuristic=1;

	read_config_file(ph, conf_file);

	switch(ph->proto){
		case 0: /*olsr*/
			ph->json_type=0;
			ph->plugin_handle = dlopen ("libprince_olsr.so", RTLD_LAZY);
		break;
		case 1: /*oonf*/
			ph->plugin_handle = dlopen ("libprince_oonf.so", RTLD_LAZY);
		break;
	}
	if(!ph->plugin_handle)
		return 0;

	new_plugin = (routing_plugin* (*)(char* host, c_graph_parser *gp, int json_type)) dlsym(ph->plugin_handle, "new_plugin");
	get_topology = (int (*)(routing_plugin *o)) dlsym(ph->plugin_handle, "get_topology");
	push_timers = (int (*)(routing_plugin *o, struct timers t)) dlsym(ph->plugin_handle, "push_timers");
	delete_plugin = (void (*)(routing_plugin *o)) dlsym(ph->plugin_handle, "delete_plugin");
	return ph;
}
/**
 * Delete a Prince handler and free all the memory
 * @param struct prince_handler* pointer to the prince_handler struct.
 */
void delete_prince_handler(struct prince_handler* ph){
	delete_plugin(ph->rp);
	bc_degree_map_delete(ph->bc_degree_map);
	dlclose(ph->plugin_handle);
	free(ph->self_id);
	free(ph->host);
	free(ph);
}


/**
 * Compute the constants needded for the timer calculation
 * and store them in ph->c
 * @param pointer to the prince_handler object
 * @return 1 if success, 0 if fail
 */
int
compute_constants(struct prince_handler *ph){
	map_id_degree_bc *m_degree_bc = ph->bc_degree_map;
	struct timers t = ph->def_t;
	int degrees=0, i;
	for(i=0; i<m_degree_bc->size;i++){
		degrees+=m_degree_bc->map[i].degree;
		/*printf("%s %f\n", m_degree_bc->map[i].id, m_degree_bc->map[i].bc);*/
	}
	ph->c.R = m_degree_bc->n_edges;
	ph->c.O_H = degrees/t.h_timer;
	ph->c.O_TC = m_degree_bc->size*ph->c.R/t.tc_timer;
	double sqrt_sum1=0, sqrt_sum2=0;
	for(i=0; i<m_degree_bc->size; i++){
		sqrt_sum1+=sqrt(m_degree_bc->map[i].degree * m_degree_bc->map[i].bc);
		sqrt_sum2+=sqrt(ph->c.R*m_degree_bc->map[i].bc);
	}
	ph->c.sq_lambda_H = sqrt_sum1/ph->c.O_H;
	ph->c.sq_lambda_TC = sqrt_sum2/ph->c.O_TC;
	return 1;
}

/**
 * Compute the timers
 * and store them in ph->opt_t
 * @param pointer to the prince_handler object
 * @return 1 if success, 0 if fail
 */
int
compute_timers(struct prince_handler *ph){
	compute_constants(ph);
	int my_index=-1, i;
	for(i=0; i<ph->bc_degree_map->size; i++){
		if(strcmp(ph->bc_degree_map->map[i].id, ph->self_id)==0){
			my_index=i;
		}
	}
	if(my_index==-1) return 0;
	ph->opt_t.h_timer = sqrt(ph->bc_degree_map->map[my_index].degree / ph->bc_degree_map->map[my_index].bc) * ph->c.sq_lambda_H;
	ph->opt_t.tc_timer = sqrt(ph->c.R/ph->bc_degree_map->map[my_index].bc)*ph->c.sq_lambda_TC;
	return 1;
}


/**
 * Read the ini configuration and populate struct prince_handler
 * @param *ph pinter to the prince_handler object
 * @param *filepath path to the configuration file
 * @return 1 if success, 0 if fail
 */
int
read_config_file(struct prince_handler *ph, char *filepath){

	if (ini_parse(filepath, handler, ph) < 0) {
	        printf("Can't load '%s'\n", filepath);
	        return 0;
	}

	printf("Config loaded from %s\n", filepath);
	return 1;
}
