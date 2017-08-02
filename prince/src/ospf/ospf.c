#include "ospf.h"
/*PUBLIC FUNCTIONS*/
/**
 * Initalize a new oonf plugin handler
 * @param host host address as a string
 * @return pointer to oonf plugin handler
 */
routing_plugin* new_plugin(char* host, int port, c_graph_parser *gp, int json_type, int timer_port){
	routing_plugin *o =(routing_plugin*) malloc(sizeof(routing_plugin));
	o->port=port;
	o->host=strdup(host);
	o->gp = gp;
	o->json_type=json_type;
  o->recv_buffer=0;
  o->self_id=0;
	o->timer_port = port;
	return o;
}

/**
 * Get the topology data from host
 * @param oonf plugin handler object
 * @return 1 if success, 0 otherwise
 */
int get_topology(routing_plugin *o)
{
	char cmd[] = "show ospf topology netjson\n";
	int c;
	bird_connect(o);
	c = write(o->sd, cmd, strlen(cmd)+1);
	if(c<0){
		printf("Write error");
		return false;
	}
	sleep(1);
	receive_netjson(o);
	o->t = parse_netjson(o->recv_buffer);
	if(!o->t){
		printf("can't parse netjson\n %s \n", o->recv_buffer );
		return false;
	}
	return true;
}

int receive_netjson(routing_plugin *o){
	char *lines[20], *token, *netjson;
	char code[5];
	int i=0,n_lines=0,lenght=0, line_len;

	if(!_telnet_receive(o->sd , &(o->recv_buffer))){
		return false;
	}
	//split the message in lines
	token = strtok(o->recv_buffer, "\n");
	while(token != NULL)
	{
		lines[i]=token;
		token = strtok(NULL, "\n");
		i++;
	}
	n_lines=i;
	//select the lines with the netjson code, remove the code and merge them
	for(i=0;i<n_lines; i++){
		strncpy(code, lines[i], 4);
		lines[i]+=5; //shift the string over the code
		if(atoi(code)==NETJSON_CODE){
			line_len = strlen(lines[i]);
			lenght += line_len;
			netjson = realloc(netjson, lenght);
			strcpy(netjson+lenght-line_len, lines[i]);
		}
	}
	free(o->recv_buffer);
	o->recv_buffer=netjson;
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
	return true;
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

void bird_connect(routing_plugin *o){
  struct sockaddr_un sa;
  o->sd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (o->sd < 0)
    printf("Cannot create socket");
  bzero(&sa, sizeof(sa));
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, o->host);
  if (connect(o->sd, (struct sockaddr *) &sa, SUN_LEN(&sa)) < 0)
    printf("Unable to connect to server control socket (%s)", o->host);
  if (fcntl(o->sd, F_SETFL, O_NONBLOCK) < 0)
    printf("fcntl");
}
