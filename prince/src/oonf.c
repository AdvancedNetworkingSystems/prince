/*
Copyright (c) 2016 Gabriele Gemmi <gabriel@autistici.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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
	return o;
}

/**
 * Get the topology data from host
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int get_topology(routing_plugin *o){

	int sd, sent;
	if((sd = _create_socket(o->host, o->port))==0){
		printf("Cannot connect to %s:%d", o->host, o->port);
		return 0;
	}
	char *req = "/netjsoninfo filter graph ipv6_0/quit\n";
	if( (sent = send(sd,req,strlen(req),MSG_NOSIGNAL))==-1){
		printf("Cannot send to %s:%d", o->host, o->port);
		close(sd);
		return 0;
	}
	
	if(!_telnet_receive(sd, &(o->recv_buffer))){
		printf("cannot receive \n");
		close(sd);
		return 0;
	}
	struct topology *t = parse_netjson(o->recv_buffer);
	if(!t){
		printf("can't parse netjson\n %s \n", o->recv_buffer);
		close(sd);
		return 0;
	}
	graph_parser_parse_simplegraph(o->gp, t);
	o->self_id=strdup(t-> self_id);
	close(sd);
	destroy_topo(t);
	return 1;
}


/**
 * Push the timers value to the routing daemon
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int push_timers(routing_plugin *o, struct timers t){
	int sd =_create_socket(o->host, o->port);
	char cmd[111];
	sprintf(cmd, "/config set olsrv2.tc_interval=%4.2f/config set interface.hello_interval=%4.2f/config commit/quit/exec=%4.6f", t.tc_timer, t.h_timer, t.exec_time);

	write(sd, cmd, strlen(cmd));

	printf("Pushed Timers %4.2f  %4.2f\n", t.tc_timer, t.h_timer);
	close(sd);
	return 1;
}

/**
 * Delete the oonf plugin handler
 * @param oonf plugin handler object
 */
void delete_plugin(routing_plugin* o){
	delete_graph_parser(o->gp);
	free(o->host);
	free(o->recv_buffer);
	free(o->self_id);
	free(o);
}

