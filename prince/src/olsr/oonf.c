#include "oonf.h"

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
	int sent;
	if((o->sd = _create_socket(o->host, o->port))==0){
		printf("Cannot connect to %s:%d", o->host, o->port);
		return 0;
	}
	char *req = "/netjsoninfo filter graph ipv6_0/quit\n";
	if( (sent = send(o->sd,req,strlen(req),MSG_NOSIGNAL))==-1){
		printf("Cannot send to %s:%d", o->host, o->port);
		close(o->sd);
		return 0;
	}
	if(o->recv_buffer!=0){
		free(o->recv_buffer);
		o->recv_buffer=0;
	}
	if(!_telnet_receive(o->sd, &(o->recv_buffer))){
		printf("cannot receive \n");
		close(o->sd);
		return 0;
	}
	o->t = parse_netjson(o->recv_buffer);
	if(!o->t){
		printf("can't parse netjson\n %s \n", o->recv_buffer);
		close(o->sd);
		return 0;
	}
	close(o->sd);
	return 1;
}


/**
 * Push the timers value to the routing daemon
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int push_timers(routing_plugin *o, struct timers t)
{
	o->sd =_create_socket(o->host, o->port);
	char cmd[111];
	sprintf(cmd, "/config set olsrv2.tc_interval=%4.2f/config set interface.hello_interval=%4.2f/config commit/quit/exec=%4.6f", t.tc_timer, t.h_timer, t.exec_time);
	write(o->sd, cmd, strlen(cmd));
	printf("Pushed Timers %4.2f  %4.2f\n", t.tc_timer, t.h_timer);
	close(o->sd);
	return 1;
}

/**
 * Delete the oonf plugin handler
 * @param oonf plugin handler object
 */
void delete_plugin(routing_plugin* o)
{
	free(o->host);
	if(o->recv_buffer!=0)
		free(o->recv_buffer);
	free(o->self_id);
	free(o);
}
