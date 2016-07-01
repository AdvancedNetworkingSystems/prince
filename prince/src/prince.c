/*
 * prince.c
 *
 *  Created on: 12 mag 2016
 *      Author: gabriel
 */
#include "prince.h"


routing_plugin* (*new_plugin)(char* host, c_graph_parser *gp, int json_type);
int (*get_topology)(routing_plugin *o);
int (*push_timers)(routing_plugin *o, struct timers t);
void (*delete_plugin)(routing_plugin* o);


int
main(int argc, char* argv[]){
	struct prince_handler *ph= new_prince_handler();
	void *handle;
	int json_type;

	if(argc>1)
		read_config_file(ph, argv[1]);
	switch(ph->proto){
		case 0: /*olsr*/
			json_type=0;
			handle = dlopen ("libprince_olsr.so", RTLD_LAZY);
		break;
		case 1: /*oonf*/
			handle = dlopen ("libprince_oonf.so", RTLD_LAZY);
		break;
	}
	if(!handle)
		return 0;

	new_plugin = (routing_plugin* (*)(char* host, c_graph_parser *gp, int json_type)) dlsym(handle, "new_plugin");
	get_topology = (int (*)(routing_plugin *o)) dlsym(handle, "get_topology");
	push_timers = (int (*)(routing_plugin *o, struct timers t)) dlsym(handle, "push_timers");
	delete_plugin = (void (*)(routing_plugin *o)) dlsym(handle, "delete_plugin");




	/*cycle each 'refresh' seconds*/
	while(true){
		ph->gp = new_graph_parser(ph->weights, ph->heuristic);
		ph->rp = new_plugin(ph->host, ph->gp, json_type);
		if(!get_topology(ph->rp)){
			delete_plugin(ph->rp);
			return 0;

		}


		graph_parser_calculate_bc(ph->gp);
		graph_parser_compose_bc_map(ph->gp, ph->bc_map);
		graph_parser_compose_degree_map(ph->gp, ph->degree_map);
		if (!compute_timers(ph))
			return 0;

		if (!push_timers(ph->rp, ph->opt_t)){
			delete_plugin(ph->rp);
			return 0;
		}
		delete_plugin(ph->rp);
		sleep(ph->refresh);
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
	int degrees=0, i;
	for(i=0; i<m_degree->size;i++){
		degrees+=m_degree->map[i].degree;
	}
	ph->c.R = m_degree->n_edges;
	ph->c.O_H = degrees/t.h_timer;
	ph->c.O_TC = m_degree->size*ph->c.R/t.tc_timer;
	double sqrt_sum1=0, sqrt_sum2=0;
	for(i=0; i<m_degree->size; i++){
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
	compute_constants(ph);
	int my_index=-1, i;
	for(i=0; i<ph->degree_map->size; i++){
		if(strcmp(ph->degree_map->map[i].id, ph->self_id)==0){
			my_index=i;
		}
	}
	if(my_index==-1) return 0;
	ph->opt_t.h_timer = sqrt(ph->degree_map->map[my_index].degree / ph->bc_map->map[my_index].bc) * ph->c.sq_lambda_H;
	ph->opt_t.tc_timer = sqrt(ph->c.R/ph->bc_map->map[my_index].bc)*ph->c.sq_lambda_TC;
	return 1;

}

static int
handler(void* user, const char* section, const char* name,
                   const char* value)
{
    struct prince_handler* pconfig = ( struct prince_handler*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    /*section :proto*/
    if (MATCH("proto", "protocol")) {
    	if(strcmp(value, "olsr")==0)	pconfig->proto = 0;
    	else if(strcmp(value, "oonf")==0)	pconfig->proto = 1;
    } else if (MATCH("proto", "host")) {
        pconfig->host = strdup(value);
    } else if (MATCH("proto", "port")) {
        pconfig->port = atoi(value);
    }else if (MATCH("proto", "self_id")) {
        pconfig->self_id = strdup(value);
    }else if (MATCH("proto", "refresh")) {
        pconfig->refresh = atoi(value);
    }
    /*section :graph-parser*/
    else if (MATCH("graph-parser", "heuristic")) {
        pconfig->heuristic = atoi(value);
    } else if (MATCH("graph-parser", "weights")) {
        pconfig->weights = atoi(value);
    }
    /* unknown section/name, error */
    else {
        return 0;
    }
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
