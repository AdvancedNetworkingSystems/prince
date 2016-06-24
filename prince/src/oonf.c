#include "oonf.h"


int	_send_telnet_cmd(int sd, char* cmd);


/*PUBLIC FUNCTIONS*/
/**
 * Initalize a new oonf plugin handler
 * @param host host address as a string
 * @return pointer to oonf plugin handler
 */
routing_plugin*
new_plugin(char* host, c_graph_parser *gp, int json_type){
	routing_plugin *o =(routing_plugin*) malloc(sizeof(routing_plugin));
	o->port=2009;
	o->host=(char*)malloc(strlen(host)*sizeof(char));
	strcpy(o->host, host);
	o->gp = gp;
	o->json_type=json_type;
	return o;
}

/**
 * Get the topology data from host
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int
get_topology(routing_plugin *o){
	int sd, sent;
	if((sd = _create_socket(o->host, o->port))==0){
		printf("Cannot connect to %s:%d", o->host, o->port);
		return 0;
	}
	char *req = "/netjsoninfo filter graph ipv6_0\n";
	if( (sent = send(sd,req,strlen(req),0))==-1){
		printf("Cannot send to %s:%d", o->host, o->port);
		return 0;
	}
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
push_timers(routing_plugin *o, struct timers t){
	int sd =_create_socket(o->host, o->port);
	char cmd[85];
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
delete_oonf_plugin(routing_plugin* o){
	delete_graph_parser(o->gp);
	free(o->host);
	free(o->recv_buffer);
	free(o);
}


/*PRIVATE FUNCTIONS */
/**
 * Write command to socket
 * @param sd socket descriptor
 * @param *cmd command string
 * @return 1 if success, 0 otherwise
 */
int
_send_telnet_cmd(int sd, char* cmd){
	/*TODO: push commands to the daemon*/
	int i;
	write(sd, cmd, strlen(cmd));
	/*for(i=0;i+=write(sd, cmd, 85);i<85);*/
	return 1;

}
