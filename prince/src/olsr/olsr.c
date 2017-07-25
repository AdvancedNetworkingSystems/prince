#include "olsr.h"

/*PUBLIC FUNCTIONS*/
/**
 * Initalize a new olsr plugin handler
 * @param host host address as a string
 * @param proto type of the remote plugin (0->netjson 1->jsoninfo)
 * @return pointer to olsr plugin handler
 */
routing_plugin* new_plugin(char* host, int port, c_graph_parser *gp, int json_type)
{
	routing_plugin *o = (routing_plugin *) malloc(sizeof(routing_plugin));
	o->port=port;
	o->host=strdup(host);
	o->gp = gp;
	o->json_type=json_type;
	return o;
}


/**
 * Get the topology data from host
 * @param olsr plugin handler object
 * @return 1 if success, 0 otherwise
 */
int get_topology(routing_plugin *o) /*netjson & jsoninfo*/
{
	int sd = _create_socket(o->host, o->port);
	char *req;
	int sent;
	switch(o->json_type){
	case 1:
		{
		/*netjson*/
		req = "/NetworkGraph";
		sent = write(sd,req,strlen(req));
		if(!_receive_data(sd, &(o->recv_buffer)))
			return 0;
		struct topology *t = parse_netjson(o->recv_buffer);
		graph_parser_parse_simplegraph(o->gp, t);
		destroy_topo(t);
		}
		break;

	case 0:
		{
		/*jsoninfo*/
		req = "/topology";
		sent = write(sd,req,strlen(req));
		if(!_receive_data(sd, &(o->recv_buffer)))
			return 0;
		struct topology *t = parse_jsoninfo(o->recv_buffer);
		graph_parser_parse_simplegraph(o->gp, t);
		destroy_topo(t);
		}
		break;

	default:
		return 0;
	}
	return 1;
}




/**
 * Push the timers value to the routing daemon
 * @param olsr plugin handler object
 * @return 1 if success, 0 otherwise
 */
int push_timers(routing_plugin *o, struct timers t)
{
	/*TODO: push h and tc value to the daemon*/
	printf("%f \t %f\n", t.h_timer, t.tc_timer);
	o->sd =_create_socket(o->host, o->port);
	char cmd[111];
	sprintf(cmd, "/HelloTimer=%4.2f/TcTimer=%4.2f", t.h_timer, t.tc_timer);
	write(o->sd, cmd, strlen(cmd));
	printf("Pushed Timers %4.2f  %4.2f\n", t.tc_timer, t.h_timer);
	close(o->sd);
	return 1;
}

/**
 * Delete the olsr plugin handler
 * @param olsr plugin handler object
 */
void delete_plugin(routing_plugin* o)
{
	delete_graph_parser(o->gp);
	free(o->host);
	free(o->recv_buffer);
	free(o);
}
