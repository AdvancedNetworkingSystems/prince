#ifndef SRC_CONFIG_GRAPH_H_
#define SRC_CONFIG_GRAPH_H_

#include <errno.h>

typedef struct graph_config * graph_config_t;

#include "config.h"
#include "prince.h"
#include "prince_handler.h"

#define GRAPH_CONFIG_SIZE    sizeof(struct proto_config)

#define INVALID_GRAPH_CONFIG NULL

struct graph_config {
        bool heuristic;
        bool weights;
        bool recursive;
        bool stop_unchanged;
        bool multithreaded;
};


graph_config_t new_graph_config(void);
void           free_graph_config(graph_config_t conf);
int            load_graph_config(const char *filepath, graph_config_t conf);


#endif /* SRC_CONFIG_GRAPH_H_ */
