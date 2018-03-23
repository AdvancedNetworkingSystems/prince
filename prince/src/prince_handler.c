#include "prince_handler.h"

#include <dlfcn.h>
#include <errno.h>
/**
* Initalize a new prince handler
* @param host host address as a string
* @return pointer to prince handler
*/
prince_handler_t new_prince_handler(char * conf_file) {
	prince_handler_t result = (prince_handler_t) malloc(PRINCE_HANDLER_SIZE);
        if (result == INVALID_PRINCE_HANDLER) {
                perror(NULL);
                return INVALID_PRINCE_HANDLER;
        }
        memset(result, 0, PRINCE_HANDLER_SIZE);
        /* ph->bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));*/
        /*setting to undefined all params*/
	result->port = -1;
	result->refresh = -1;
	result->sleep_onfail = 1;

	if (read_config_file(result, conf_file) == 0) {
		return INVALID_PRINCE_HANDLER;
        }

        if (load_routing_plugin(result) != 0) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "new_plugin") != 0) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "delete_plugin") != 0) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "get_initial_timers") != 0) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "push_timers") != 0) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
        if (load_routing_plugin_symbol(result, "get_topology") != 0) {
                perror("prince-plugin");
                return INVALID_PRINCE_HANDLER;
        }
	return result;
}

/**
* Delete a Prince handler and free all the memory
* @param struct prince_handler* pointer to the prince_handler struct.
*/
void free_prince_handler(prince_handler_t ph)
{
	free_graph_parser(ph->gp);
	delete_plugin_p(ph->rp);
	dlclose(ph->plugin_handle);
	char* tmp=dlerror();
	free(tmp);
	/*bc_degree_map_delete(ph->bc_degree_map);*/
	free(ph->self_id);
	free(ph->host);
	free(ph);
}
