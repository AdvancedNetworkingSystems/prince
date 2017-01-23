/* 
 * File:   biconnected.h
 * Author: mb03
 *
 * Created on January 18, 2017, 12:19 PM
 */

#ifndef BICONNECTED_H
#define BICONNECTED_H
#include "graph/graph.h"
#ifdef __cplusplus
extern "C" {
#endif
    struct graph;
    
    struct connected_component{
        struct graph g;
        int * mapping;
        int * weights;
        struct node_graph * cutpoint;
        int cutpoint_index;
    };
    
    struct sub_graph{
        struct list connected_components;
        int size;
    };
    
    //These function returns the list of list of connected components
    //i.e. a list of connected components for each connected subgraph
    struct list*  tarjan_rec_undir(struct graph * g, 
            bool * is_articulation_point, int * component_indexes);
    struct list*  tarjan_iter_undir(struct graph * g, 
            bool * is_articulation_point, int * component_indexes);
    
    //not employed  and completed yet (missing art_poit and 
    //division in subgraph with componenent indexing)
    struct list*  tarjan_rec_dir(struct graph * g);
    struct list*  tarjan_iter_dir(struct graph * g);
    
    
#ifdef __cplusplus
}
#endif

#endif /* BICONNECTED_H */

