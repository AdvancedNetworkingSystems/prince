#include "config_graph.h"

int load_graph_config(const char *filepath, graph_config_t conf) {
        if (conf == INVALID_GRAPH_CONFIG) {
                return -1;
        }
        prince_handler_t temp = new_prince_handler(filepath);
        if (temp == INVALID_PRINCE_HANDLER) {
                return -1;
        }
        conf->heuristic      = temp->heuristic;
        conf->weights        = temp->weights;
        conf->recursive      = temp->recursive;
        conf->stop_unchanged = temp->stop_unchanged;
        conf->multithreaded  = temp->multithreaded;
        return 0;
}

graph_config_t new_graph_config(void) {
        graph_config_t result = (graph_config_t) malloc(GRAPH_CONFIG_SIZE);
        if (result == INVALID_GRAPH_CONFIG) {
                perror(NULL);
                return INVALID_GRAPH_CONFIG;
        }
        memset(result, 0, GRAPH_CONFIG_SIZE);
        return result;
}

void free_graph_config(graph_config_t conf) {
        free(conf);
}
