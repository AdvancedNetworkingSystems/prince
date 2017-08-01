#include "prince.h"

routing_plugin* (*new_plugin_p)(char* host, int port, c_graph_parser *gp, int json_type);
int (*get_topology_p)(routing_plugin *o);
int (*push_timers_p)(routing_plugin *o, struct timers t);
void (*delete_plugin_p)(routing_plugin* o);

/**
* Main routine of Prince. Collect topology, parse it, calculate bc and timers, push them back.
* @param argv[1] <- config filename
* @return 1 on success, 0 on error
*/
int main(int argc, char* argv[])
{
	if(argc<2){
		printf("No conf file specified. Exiting.\n");
		return -1;
	}
	printf("Prince Started\n");
	struct prince_handler *ph= new_prince_handler(argv[1]);
	if(ph==0)
		return -1;

	recursive=ph->recursive;
	multithread=ph->multithreaded;
	stop_computing_if_unchanged=ph->stop_unchanged;
	ph->gp = new_graph_parser(ph->weights, ph->heuristic);
	int go = 1;
	struct graph_parser * gp_p=(struct graph_parser *)ph->gp ;
	ph->rp = new_plugin_p(ph->host, ph->port, ph->gp, ph->json_type);
	do{
		sleep(ph->refresh);
		if(!get_topology_p(ph->rp)){
			printf("Error getting topology\n");
			sleep(ph->sleep_onfail);
			continue;
		}
		printf("Got Topology\n");
		graph_parser_parse_simplegraph(ph->rp->gp, ph->rp->t);
		if(ph->rp->self_id!=0)
			free(ph->rp->self_id);
		ph->rp->self_id=strdup(ph->rp->t->self_id);

		if(ph->rp->self_id){
			if(ph->self_id!=0)
			free(ph->self_id);
			ph->self_id = strdup(ph->rp->self_id);
		}
		clock_t start = clock();
		graph_parser_calculate_bc(ph->gp);
		clock_t end = clock();
		ph->bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));
		ph->bc_degree_map->size=gp_p->g.nodes.size;
		ph->bc_degree_map->map=0;
		graph_parser_compose_degree_bc_map(ph->gp, ph->bc_degree_map);
		ph->opt_t.exec_time = (double)(end - start) / CLOCKS_PER_SEC;
		printf("\nCalculation time: %fs\n", ph->opt_t.exec_time);
		if (!compute_timers(ph)){
			delete_prince_handler(ph);
			continue;
		}
		printf("Id of the node we are computing is: %s\n", ph->self_id);
		if (!push_timers_p(ph->rp, ph->opt_t)){
			delete_prince_handler(ph);
			continue;
		}
		free(gp_p->bc);
		gp_p->bc=0;
		bc_degree_map_delete(ph->bc_degree_map);
		free_graph(&(gp_p->g));
		init_graph(&(gp_p->g));
	}while(go);
	delete_prince_handler(ph);
	printf("Prince Exited\n");
	return 0;
}

/**
* Initalize a new prince handler
* @param host host address as a string
* @return pointer to prince handler
*/
struct prince_handler* new_prince_handler(char * conf_file)
{
	struct prince_handler* ph = (struct prince_handler*) malloc(sizeof(struct prince_handler));
	ph->def_t.h_timer=2.0;
	ph->def_t.tc_timer=5.0;
	/* ph->bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));*/
	/*setting to undefined all params*/
	ph->proto=-1;
	ph->host=0;
	ph->port=-1;
	ph->refresh=-1;
	ph->self_id=0;
	ph->sleep_onfail = 1;
	if(read_config_file(ph, conf_file)==0)
		return 0;
	switch(ph->proto){
		case 0: /*olsr*/
		ph->json_type=1;
		ph->plugin_handle = dlopen ("libprince_olsr.so", RTLD_LAZY);
		break;
		case 1: /*oonf*/
		ph->plugin_handle = dlopen ("libprince_oonf.so", RTLD_LAZY);
		break;
		case 2: /*ospf*/
		ph->plugin_handle = dlopen ("libprince_ospf.so", RTLD_LAZY);
		break;
	}
	if(!ph->plugin_handle)
		return 0;
	new_plugin_p = (routing_plugin* (*)(char* host, int port, c_graph_parser *gp, int json_type)) dlsym(ph->plugin_handle, "new_plugin");
	get_topology_p = (int (*)(routing_plugin *o)) dlsym(ph->plugin_handle, "get_topology");
	push_timers_p = (int (*)(routing_plugin *o, struct timers t)) dlsym(ph->plugin_handle, "push_timers");
	delete_plugin_p = (void (*)(routing_plugin *o)) dlsym(ph->plugin_handle, "delete_plugin");
	return ph;
}
/**
* Delete a Prince handler and free all the memory
* @param struct prince_handler* pointer to the prince_handler struct.
*/
void delete_prince_handler(struct prince_handler* ph)
{
	delete_graph_parser(ph->gp);
	delete_plugin_p(ph->rp);
	dlclose(ph->plugin_handle);
	char* tmp=dlerror();
	free(tmp);
	/*bc_degree_map_delete(ph->bc_degree_map);*/
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
int compute_constants(struct prince_handler *ph)
{
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
int compute_timers(struct prince_handler *ph)
{
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
