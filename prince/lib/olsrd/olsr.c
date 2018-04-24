#include "olsr.h"

/*PUBLIC FUNCTIONS*/
/**
 * Initalize a new olsr plugin handler
 * @param host host address as a string
 * @param proto type of the remote plugin (0->netjson 1->jsoninfo)
 * @return pointer to olsr plugin handler
 */
routing_plugin *new_plugin(char *host, int port, int json_type, int timer_port)
{
	routing_plugin *o = (routing_plugin *)malloc(sizeof(routing_plugin));
	if (o == NULL) {
		perror("olsr-new");
		return NULL;
	}
	o->port = port;
	o->host = strdup(host);
	o->recv_buffer = 0;
	o->self_id = 0;
	o->json_type = json_type;
	o->timer_port = timer_port;
	return o;
}
int get_initial_timers(routing_plugin *o, struct timers *t)
{
	t->h_timer = parse_initial_timer(o, HELLO_TIMER_MESSAGE);
	t->tc_timer = parse_initial_timer(o, TC_TIMER_MESSAGE);
	if (t->h_timer == -1) {
		fprintf(stderr, "Could not initialise h_timer\n");
		fprintf(stdout, "Setting h_timer to 2\n");
		t->h_timer = 2;
	}
	if (t->tc_timer == -1) {
		fprintf(stderr, "Could not initialise tc_timer\n");
		fprintf(stdout, "Setting tc_timer to 5\n");
		t->tc_timer = 5;
	}
	return 0;
}

float parse_initial_timer(routing_plugin *o, char *cmd)
{
	o->sd = _create_socket(o->host, o->timer_port, ECONNREFUSED);
	char *page;
	char *token;
	float value = 0;
	page = (char *)malloc(RESPONSE_SIZE);
	if (page == NULL) {
		perror("olsr");
		return -1;
	}
	write(o->sd, cmd, strlen(cmd));
	if (recv(o->sd, page, RESPONSE_SIZE, 0) > 0) {
		token = strtok(page, ":");
		token = strtok(NULL, ":");
		value = atof(token);
	} else {
		fprintf(stderr, "Could not recieve timer %s\n", cmd);
		return -1;
	}
	close(o->sd);
	free(page);
	if (value == 0) {
		return -1;
	}
	return value;
}

/**
 * Get the topology data from host
 * @param olsr plugin handler object
 * @return 0 if success, -1 otherwise
 */
int get_topology(routing_plugin *o) /*netjson & jsoninfo*/
{
	int sent;
	if ((o->sd = _create_socket(o->host, o->port, 0)) == 0) {
		printf("Cannot connect to %s:%d", o->host, o->port);
		return -1;
	}
	switch (o->json_type) {
	case 0: {
		/*olsrd jsoninfo*/
		char *req = "/topology/config";
		if ((sent = send(o->sd, req, strlen(req), MSG_NOSIGNAL))
		    == -1) {
			printf("Cannot send to %s:%d", o->host, o->port);
			close(o->sd);
			return -1;
		}
		if (o->recv_buffer != 0) {
			free(o->recv_buffer);
			o->recv_buffer = 0;
		}
		if (!_telnet_receive(o->sd, &(o->recv_buffer))) {
			printf("cannot receive \n");
			close(o->sd);
			return -1;
		}
		o->t = parse_jsoninfo(o->recv_buffer);
		if (!o->t) {
			printf("can't parse jsoninfo\n %s \n", o->recv_buffer);
			close(o->sd);
			return -1;
		}
	} break;
	case 1: {
		/*olsrd netjson*/
		char *req = "/NetworkGraph";
		if ((sent = send(o->sd, req, strlen(req), MSG_NOSIGNAL))
		    == -1) {
			printf("Cannot send to %s:%d\n", o->host, o->port);
			close(o->sd);
			return -1;
		}
		if (o->recv_buffer != 0) {
			free(o->recv_buffer);
			o->recv_buffer = 0;
		}
		if (!_telnet_receive(o->sd, &(o->recv_buffer))) {
			printf("cannot receive \n");
			close(o->sd);
			return -1;
		}
		o->t = parse_netjson(o->recv_buffer);
		if (!o->t) {
			printf("can't parse netjson\n %s \n", o->recv_buffer);
			close(o->sd);
			return -1;
		}
	} break;
	default:
		close(o->sd);
		return -1;
	}
	close(o->sd);
	return 0;
}


/**
 * Push the timers value to the routing daemon
 * @param olsr plugin handler object
 * @return 1 if success, 0 otherwise
 */
int push_timers(routing_plugin *o, struct timers t)
{
	o->sd = _create_socket(o->host, o->timer_port, 0);
	char cmd[25];
	sprintf(cmd, "/HelloTimer=%4.4f", t.h_timer);
	write(o->sd, cmd, strlen(cmd));
	close(o->sd);
	o->sd = _create_socket(o->host, o->timer_port, 0);
	sprintf(cmd, "/TcTimer=%4.4f", t.tc_timer);
	write(o->sd, cmd, strlen(cmd));
	printf("%4.4f\t%4.4f\t%4.4f\t%4.4f\n", t.tc_timer, t.h_timer,
	       t.exec_time, t.centrality);
	close(o->sd);
	return 1;
}

/**
 * Delete the olsr plugin handler
 * @param olsr plugin handler object
 */
void delete_plugin(routing_plugin *o)
{
	free(o->host);
	if (o->recv_buffer != 0)
		free(o->recv_buffer);
	free(o->self_id);
	free(o);
}
