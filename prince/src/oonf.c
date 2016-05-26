#include "oonf.h"

//PUBLIC FUNCTIONS
/**
 * Initalize a new oonf plugin handler
 * @param host host address as a string
 * @return pointer to oonf plugin handler
 */
oonf_routing_plugin*
new_oonf_plugin(char* host, c_graph_parser *gp){
	oonf_routing_plugin *o =(oonf_routing_plugin*) malloc(sizeof(oonf_routing_plugin));
	o->port=2009;
	o->host=(char*)malloc(strlen(host)*sizeof(char));
	strcpy(o->host, host);
	o->gp = gp;
	return o;
}

/**
 * Get the topology data from host
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int
get_netjson_topology(oonf_routing_plugin *o){
	int sd = _create_socket(o->host, o->port);
	char *req = "/netjsoninfo filter graph ipv6_0\n";
	int sent = send(sd,req,strlen(req),0);
	if(!_telnet_receive(sd, &(o->recv_buffer))){
		return 0;
	}
	graph_parser_parse_netjson(o->gp, o->recv_buffer);

	close(sd);
	return 1;
}


/**
 * Push the timers value to the routing daemon
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int
oonf_push_timers(oonf_routing_plugin *o, struct timers t){
	int sd =_create_socket(o->host, o->port);
	char cmd[100];
	sprintf(cmd, "/config set olsrv2.tc_timer=%4.2f/config set interface.hello_timer=%4.2f/config commit", t.tc_timer, t.h_timer);
	if(!_send_telnet_cmd(sd, cmd))
		return 0;
	return 1;
}

/**
 * Delete the oonf plugin handler
 * @param oonf plugin handler object
 */
void
delete_oonf_plugin(oonf_routing_plugin* o){
	delete_graph_parser(o->gp);
	free(o->host);
	free(o->recv_buffer);
	free(o);
}


//PRIVATE FUNCTIONS
/**
 * Write command to socket
 * @param sd socket descriptor
 * @param *cmd command string
 * @return 1 if success, 0 otherwise
 */
int
_send_telnet_cmd(int sd, char* cmd){
	//TODO: push commands to the daemon
	int i;
	for(i=0;i+=write(sd, cmd, strlen(cmd));i<strlen(cmd));
	return 1;

}
