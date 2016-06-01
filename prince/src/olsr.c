#include "olsr.h"

//PUBLIC FUNCTIONS
/**
 * Initalize a new olsr plugin handler
 * @param host host address as a string
 * @param proto type of the remote plugin (0->netjson 1->jsoninfo)
 * @return pointer to olsr plugin handler
 */
olsr_routing_plugin*
new_olsr_plugin(char* host, c_graph_parser *gp, int proto){
	olsr_routing_plugin *o = (olsr_routing_plugin *) malloc(sizeof(olsr_routing_plugin));
	switch(proto){
		case 0:
			o->port=2005;
		break;

		case 1:
			o->port=9090;
		break;
	}
	o->proto=proto;
	o->host=(char*)malloc(strlen(host)*sizeof(char));
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
get_olsr_topology(olsr_routing_plugin *o){
	int sd = _create_socket(o->host, o->port);
	char *req;
	int sent;
	switch(o->proto){
	case 0:
		//netjson
		req = "/NetworkGraph";
		sent = write(sd,req,strlen(req));
		if(!_receive_data(sd, &(o->recv_buffer))){
			return 0;
		}
		graph_parser_parse_netjson(o->gp, o->recv_buffer);
		break;

	case 1:
		req = "/topology";
		sent = write(sd,req,strlen(req));
		if(!_receive_data(sd, &(o->recv_buffer))){
			return 0;
		}
		graph_parser_parse_jsoninfo(o->gp, o->recv_buffer);
		break;
	}
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
