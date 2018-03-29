#include "prince_handler.h"

#include <dlfcn.h>
#include <errno.h>
/**
* Initalize a new prince handler
* @param host host address as a string
* @return pointer to prince handler
*/
prince_handler_t new_prince_handler(const char * conf_file) {
	prince_handler_t result = (prince_handler_t) malloc(PRINCE_HANDLER_SIZE);
        if (result == INVALID_PRINCE_HANDLER) {
                perror(NULL);
                return INVALID_PRINCE_HANDLER;
        }
        memset(result, 0, PRINCE_HANDLER_SIZE);
        /* ph->bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));*/

        result->proto_config = new_proto_config();
        if (result->proto_config == INVALID_PROTO_CONFIG) {
                free_prince_handler(result);
                return INVALID_PRINCE_HANDLER;
        }

        result->graph_config = new_graph_config();
        if (result->graph_config == INVALID_PROTO_CONFIG) {
                free_prince_handler(result);
                return INVALID_PRINCE_HANDLER;
        }

        /*setting to undefined all params*/
	result->port = -1;
	result->refresh = -1;
	result->sleep_onfail = 1;

	if (read_config_file(result, conf_file)) {
		return INVALID_PRINCE_HANDLER;
        }

	result->gp = new_graph_parser(result->weights, result->heuristic);
        if (result->gp == NULL) {
                fprintf(stderr, "Could not create graph_parser\n");
                free_prince_handler(result);
                exit(EXIT_FAILURE);
        }

        if (load_routing_plugin(result)) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "new_plugin")) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "delete_plugin")) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "get_initial_timers") != 0) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "push_timers")) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "get_topology")) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
	return result;
}

/**
* Delete a Prince handler and free all the memory
* @param struct prince_handler* pointer to the prince_handler struct.
*/
int free_prince_handler(prince_handler_t ph) {
	free_graph_parser(ph->gp);
        free_graph_config(ph->graph_config);
        free_proto_config(ph->proto_config);
        if (free_routing_plugin(ph)) {
                perror("prince-handler");
                errno = ELIBACC;
                return -1;
        }
	/*bc_degree_map_delete(ph->bc_degree_map);*/
	free(ph->self_id);
	free(ph->host);
	free(ph);
        return 0;
}
