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

#ifdef __cplusplus
extern "C" {
#endif
struct graph;

struct connected_component{
    struct list *nodes;
    int id;
};

struct list*  tarjan_rec(struct graph * g);
struct list*  tarjan_iter(struct graph * g);
double * betweeness_brandes(struct graph * g);


#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_H */

