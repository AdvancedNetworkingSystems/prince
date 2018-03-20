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

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Structs and functions used to represent a graph. Further detail in grap.c
     */
    struct node_graph
    {
        struct list neighbours;


        char * name;
        int    node_graph_id;
    };


    struct edge_graph
    {
        struct node_graph *to;


        double value;
    };


    struct graph
    {
        struct list nodes;


        bool directed;
    };


    void init_graph(struct graph *g);

    void add_edge_graph(struct graph *g,
                        const char * name_from,
                        const char * name_to,
                        double       value,
                        bool         directed);

    void add_edge_graph_return_node_indexes(struct graph *g,
            const char *                                 name_from,
            const char *                                 name_to,
            double                                       value,
            bool                                         directed,
            int *                                        nodefrom,
            int *                                        nodeto);

    void free_graph(struct graph *g);

#ifdef __cplusplus
}
#endif

#endif /* GRAPH_H */


