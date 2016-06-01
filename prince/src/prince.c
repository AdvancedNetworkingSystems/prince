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
	if(argc>1)
		read_config_file(ph, argv[1]);
	ph->gp = new_graph_parser(ph->weights, ph->heuristic);
	switch(ph->rp){
	case 0: //olsr
		ph->olsr_rp = new_olsr_plugin(ph->host, ph->gp, 1);
		if(!get_olsr_topology(ph->olsr_rp))
			return 0;
	break;
	case 1: //oonf
		ph->oonf_rp = new_oonf_plugin(ph->host, ph->gp);
		if(!get_netjson_topology(ph->oonf_rp))
			return 0;
	break;
	}
	graph_parser_calculate_bc(ph->gp);
	graph_parser_compose_bc_map(ph->gp, ph->bc_map);
	graph_parser_compose_degree_map(ph->gp, ph->degree_map);
	compute_constants(ph);
	compute_timers(ph);


	switch(ph->rp){
		case 0: //olsr
			olsr_push_timers(ph->olsr_rp, ph->opt_t);
		break;

		case 1: //oonf
			oonf_push_timers(ph->oonf_rp, ph->opt_t);
		break;
		}

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
	ph->weights=0;
	ph->heuristic=1;
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
		sqrt_sum2+=sqrt(ph->c.R*m_bc->map[i].bc);
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
	ph->opt_t.h_timer = sqrt(ph->degree_map->map[my_index].degree / ph->bc_map->map[my_index].bc) * ph->c.sq_lambda_H;
	ph->opt_t.tc_timer = sqrt(ph->c.R/ph->bc_map->map[my_index].bc)*ph->c.sq_lambda_TC;
	return 1;

}


/**
 * Read the configuration from a file and load it in the prince handler struct
 * @param *ph pinter to the prince_handler object
 * @param *filepath path to the configuration file
 * @return 1 if success, 0 if fail
 */
int read_config_file(struct prince_handler *ph, char *filepath){
	FILE* config;
	char lb[LINE_SIZE], lb2[LINE_SIZE];
	int val;
	config = fopen(filepath, "r");
	int params=0;

	while(fscanf(config, "%s %s\n", &lb, &lb2)>0){
		if(strcmp(lb, "protocol" )==0){
			if(strcmp(lb2, "olsr")==0){
				ph->rp=0;
				params++;
			}else if(strcmp(lb2, "oonf")==0){
				ph->rp=1;
				params++;
		}
		}else if(strcmp(lb, "host" )==0){
			ph->host=malloc(strlen(lb2)*sizeof(char));
			strcpy(ph->host, lb2);//SHould i malloc it?
			params++;

		}else if(strcmp(lb, "self_id")==0){
			ph->self_id=malloc(strlen(lb2)*sizeof(char));
			strcpy(ph->self_id, lb2);
			params++;
		}else if(strcmp(lb, "heuristic")==0){
			if(strcmp(lb2, true)) ph->heuristic=1;
			if(strcmp(lb2, false)) ph->heuristic=0;
		}else if(strcmp(lb, "weights")==0){
			if(strcmp(lb2, true)) ph->weights=1;
			if(strcmp(lb2, false)) ph->weights=0;
		}
	}
	if(params<4) return 0; //check if the parametes are setted
	return 1;

}
