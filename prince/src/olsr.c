#include "olsr.h"

//PUBLIC FUNCTIONS
/**
 * Initalize a new olsr plugin handler
 * @param host host address as a string
 * @return pointer to olsr plugin handler
 */
olsr_routing_plugin*
new_olsr_plugin(char* host, c_graph_parser *gp){
	olsr_routing_plugin *o = (olsr_routing_plugin *) malloc(sizeof(olsr_routing_plugin));
	o->port=9090;
	o->host=(char*)malloc(strlen(host)*sizeof(char));
	//o->bc_map = (map_id_bc_pair*)malloc(sizeof(map_id_bc_pair));
	//o->degree_map = (map_id_degree_pair*)malloc(sizeof(map_id_degree_pair));
	strcpy(o->host, host);
	o->gp = gp;
	return o;

}

/**
 * Get the topology data from host
 * @param olsr plugin handler object
 * @return 1 if success, 0 otherwise
 */
int
get_jsoninfo_topology(olsr_routing_plugin *o){
	int sd = _create_socket(o->host, o->port);
	char *req = "/topology";
	int sent = write(sd,req,strlen(req));
	if(!_receive_data(sd, &(o->recv_buffer))){
		return 0;
	}
	graph_parser_parse_jsoninfo(o->gp, o->recv_buffer);
	return 1;
}




/**
 * Push the timers value to the routing daemon
 * @param olsr plugin handler object
 * @return 1 if success, 0 otherwise
 */
int
olsr_push_timers(olsr_routing_plugin *o, struct timers t){
	//TODO: push h and tc value to the daemon
	return 0;
}

/**
 * Delete the olsr plugin handler
 * @param olsr plugin handler object
 */
void
delete_olsr_plugin(olsr_routing_plugin* o){
	delete_graph_parser(o->gp);
	free(o->host);
	free(o->recv_buffer);
	free(o);
}
