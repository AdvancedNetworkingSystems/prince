/*
 * File:   graph.h
 * Author: mb03
 *
 * Created on December 7, 2016, 3:48 PM
 */

#ifndef GRAPH_H
#define GRAPH_H

#include "list.h"
#include <stdbool.h>
#include <string.h>

#define INVALID_NODE_GRAPH NULL
#define INVALID_EDGE_GRAPH NULL
#define INVALID_GRAPH      NULL
#define NODE_GRAPH_SIZE    sizeof(struct node_graph)
#define EDGE_GRAPH_SIZE    sizeof(struct edge_graph)
#define GRAPH_SIZE         sizeof(struct graph)

/**
* Structs and functions used to represent a graph. Further detail in grap.c
*/
struct node_graph {
        struct list neighbours;
        char * name;
        int    node_graph_id;
};

typedef struct node_graph * node_graph_t;


struct edge_graph {
        struct node_graph *to;
        double value;
};

typedef struct edge_graph * edge_graph_t;


struct graph {
        struct list nodes;
        bool directed;
};

typedef struct graph * graph_t;


void init_graph(graph_t g);

void add_edge_graph(graph_t g,
                const char * name_from,
                const char * name_to,
                double       value,
                bool         directed);

void add_edge_graph_return_node_indexes(graph_t g,
    const char *                                 name_from,
    const char *                                 name_to,
    double                                       value,
    bool                                         directed,
    int *                                        nodefrom,
    int *                                        nodeto);

void free_graph(graph_t g);

#endif /* GRAPH_H */
