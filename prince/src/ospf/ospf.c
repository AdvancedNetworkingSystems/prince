#include "ospf.h"

/*PUBLIC FUNCTIONS*/
/**
 * Initalize a new oonf plugin handler
 * @param host host address as a string
 * @return pointer to oonf plugin handler
 */
routing_plugin* new_plugin(char* host, int port, c_graph_parser *gp, int json_type){
	routing_plugin *o =(routing_plugin*) malloc(sizeof(routing_plugin));
	o->port=port;
	o->host=strdup(host);
	o->gp = gp;
	o->json_type=json_type;
        o->recv_buffer=0;
        o->self_id=0;
	return o;
}

/**
 * Get the topology data from host
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int get_topology(routing_plugin *o)
{
	bird_connect("/var/run/bird.ctl");
	server_send("SHOW OSPF TOPOLOGY NETJSON");
	server_read();
  //Fill o->recv_buffer with netjson

	struct topology *t = parse_netjson(o->recv_buffer);
	if(!t){
		printf("can't parse netjson\n %s \n", o->recv_buffer);
		return 0;
	}
	graph_parser_parse_simplegraph(o->gp, t);
        if(o->self_id!=0)
            free(o->self_id);
	o->self_id=strdup(t-> self_id);
	destroy_topo(t);
	return 1;
}


/**
 * Push the timers value to the routing daemon
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int push_timers(routing_plugin *o, struct timers t)
{

}

/**
 * Delete the oonf plugin handler
 * @param oonf plugin handler object
 */
void delete_plugin(routing_plugin* o)
{
	delete_graph_parser(o->gp);
	free(o->host);
        if(o->recv_buffer!=0)
            free(o->recv_buffer);
	free(o->self_id);
	free(o);
}
