/*
 * prince.c
 *
 *  Created on: 12 mag 2016
 *      Author: gabriel
 */
#include "prince.h"


int
main(int argc, char* argv[]){
	struct prince_handler *ph= new_prince_handler();
	ph->self_id="10.150.13.4";
	ph->olsr_rp = new_olsr_plugin("79.52.131.119", ph->gp);
	get_jsoninfo_topology(ph->olsr_rp);

	graph_parser_calculate_bc(ph->gp);
	graph_parser_compose_bc_map(ph->gp, ph->bc_map);
	graph_parser_compose_degree_map(ph->gp, ph->degree_map);
	compute_constants(ph);
	compute_timers(ph);
	olsr_push_timers(ph->olsr_rp, ph->opt_t);
	return 1;

}

/**
 * Initalize a new prince handler
 * @param host host address as a string
 * @return pointer to prince handler
 */
struct prince_handler*
new_prince_handler(){
	struct prince_handler* ph = (struct prince_handler*) malloc(sizeof(struct prince_handler));
	ph->def_t.h_timer=2.0;
	ph->def_t.tc_timer=5.0;
	ph->degree_map = (map_id_degree_pair *) malloc(sizeof(map_id_degree_pair));
	ph->bc_map = (map_id_bc_pair *) malloc(sizeof(map_id_bc_pair));
	ph->gp = new_graph_parser(true, false);
	return ph;
}

/**
 * Compute the constants needded for the timer calculation
 * and store them in ph->c
 * @param pointer to the prince_handler object
 * @return 1 if success, 0 if fail
 */
int
compute_constants(struct prince_handler *ph){
	map_id_degree_pair *m_degree = ph->degree_map;
	map_id_bc_pair *m_bc = ph->bc_map;
	struct timers t = ph->def_t;
	int degrees=0;
	for(int i=0; i<m_degree->size;i++){
		degrees+=m_degree->map[i].degree;
	}
	ph->c.R = m_degree->n_edges;
	ph->c.O_H = degrees/t.h_timer;
	ph->c.O_TC = m_degree->size*ph->c.R/t.tc_timer;
	double sqrt_sum1=0, sqrt_sum2=0;
	for(int i=0; i<m_degree->size; i++){
		sqrt_sum1+=sqrt(m_degree->map[i].degree * m_bc->map[i].bc);
		sqrt_sum2+=sqrt(ph->c.R*m_degree->map[i].degree);
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
	int my_index=-1;
	for(int i=0; i<ph->degree_map->size; i++){
		if(strcmp(ph->degree_map->map[i].id, ph->self_id)==0){
			my_index=i;
		}
	}
	if(my_index==-1) return 0;
	ph->opt_t.h_timer = sqrt(ph->degree_map->map[my_index].degree * ph->bc_map->map[my_index].bc) * ph->c.sq_lambda_H;
	//ph->opt_t.h_timer = sqrt((ph->degree_map->map[my_index].degree)*(ph->bc_map->map[my_index].bc))*(ph->c.sq_lambda_H);
	ph->opt_t.tc_timer = sqrt(ph->c.R*ph->bc_map->map[my_index].bc)*ph->c.sq_lambda_TC;
	return 1;

}

