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
#include "olsr.h"

/*PUBLIC FUNCTIONS*/
/**
 * Initalize a new olsr plugin handler
 * @param host host address as a string
 * @param proto type of the remote plugin (0->netjson 1->jsoninfo)
 * @return pointer to olsr plugin handler
 */
#ifdef unique
routing_plugin* new_plugin_olsr(char* host, int port, c_graph_parser *gp, int json_type){
#else
routing_plugin* new_plugin(char* host, int port, c_graph_parser *gp, int json_type){
#endif
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
#ifdef unique
int get_topology_olsr(routing_plugin *o){
#else
int get_topology(routing_plugin *o){ /*netjson & jsoninfo*/
#endif
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
#ifdef unique
int push_timers_olsr(routing_plugin *o, struct timers t){
#else
int push_timers(routing_plugin *o, struct timers t){
#endif
	/*TODO: push h and tc value to the daemon*/
	printf("%f \t %f\n", t.h_timer, t.tc_timer);
	return 1;
}

/**
 * Delete the olsr plugin handler
 * @param olsr plugin handler object
 */
#ifdef unique
void delete_plugin_olsr(routing_plugin* o){
#else
void delete_plugin(routing_plugin* o){
#endif
	delete_graph_parser(o->gp);
	free(o->host);
	free(o->recv_buffer);
	free(o);
}


