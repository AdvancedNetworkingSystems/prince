/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

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
        
        bool added;
        
        struct node_graph * cutpoint;
        int cutpoint_index;
    };
    
    struct list*  tarjan_rec_dir(struct graph * g);
    struct list*  tarjan_iter_dir(struct graph * g);
    struct list*  tarjan_rec_undir(struct graph * g, bool * is_articulation_point);
    struct list*  tarjan_iter_undir(struct graph * g, bool * is_articulation_point);
    
#ifdef __cplusplus
}
#endif

#endif /* BICONNECTED_H */

