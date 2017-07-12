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
	int server_fd;
	struct topology *t;
	char cmd[] = "show ospf topology netjson\n";
	int c;
	bird_connect(&server_fd, o->host);
	c = write(server_fd, cmd, strlen(cmd)+1);
	if(c<0){
		printf("Write error");
		return false;
	}
	sleep(1);
	if(!_telnet_receive(server_fd , &(o->recv_buffer))){
		return false;
	}
	char * json_beg = strstr(o->recv_buffer, "\n")+6;
	t = parse_netjson(json_beg);
	if(!t){
		printf("can't parse netjson\n %s \n", json_beg);
		return false;
	}
	graph_parser_parse_simplegraph(o->gp, t);
  if(o->self_id!=0)
    free(o->self_id);
	o->self_id=strdup(t-> self_id);
	destroy_topo(t);
	return true;
}


/**
 * Push the timers value to the routing daemon
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int push_timers(routing_plugin *o, struct timers t)
{
	//TODO
	return false;
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

void bird_connect(int *server_fd, char *server_path){
  struct sockaddr_un sa;
  *server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (*server_fd < 0)
    printf("Cannot create socket");

  bzero(&sa, sizeof(sa));
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, server_path);
  if (connect(*server_fd, (struct sockaddr *) &sa, SUN_LEN(&sa)) < 0)
    printf("Unable to connect to server control socket (%s)", server_path);
  if (fcntl(*server_fd, F_SETFL, O_NONBLOCK) < 0)
    printf("fcntl");
}
