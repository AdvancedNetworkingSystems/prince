/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   algorithms.h
 * Author: mb03
 *
 * Created on December 12, 2016, 5:55 PM
 */

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "graph/graph.h"
#include <pthread.h>
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

double * betweeness_brandes(struct graph * g, bool endpoints,int ** traffic_matrix);
double * betwenness_heuristic(struct graph * g, bool recursive);


#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_H */

