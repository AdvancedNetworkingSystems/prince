/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   graph.h
 * Author: principale
 *
 * Created on December 7, 2016, 3:48 PM
 */

#ifndef GRAPH_H
#define GRAPH_H

#include "list.h" 

#ifdef __cplusplus
extern "C" {
#endif
    
    struct node_graph {
        struct queue neighbours;
        const char * name;
        
    }; 
    struct edge_graph {
        struct node_graph * to;
        double value;
        
    }; 
    struct graph {
        struct queue nodes;
    };
    
    void init_graph(struct graph * g);
    struct node_graph * add_node_graph(struct graph * g, const char * name);
    void add_edge_graph(struct graph * g, const char * name_from, const char * name_to, double value);
    void print_graph(struct graph * g);
    
    void init_node_graph(struct node_graph * n,const char * name);
    void init_edge_graph(struct edge_graph * e);
    void init_edge_graph_params(struct edge_graph * e,struct node_graph * to,double value);

    
#ifdef __cplusplus
}
#endif

#endif /* GRAPH_H */

