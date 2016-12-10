/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

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
#ifdef __cplusplus
extern "C" {
#endif
    
    
    struct node_graph {
        struct list neighbours;
        const char * name;
        //for recursive version
        int index;
        int low_link;
        bool on_stack;
        int bcc_id;
        //for iterative version
        struct node_graph * caller;
        struct node_list * iterator;
    }; 
    struct edge_graph {
        struct node_graph * to;
        double value;
        
    }; 
    struct graph {
        struct list nodes;
    };
    
    void init_graph(struct graph * g);
    struct node_graph * add_node_graph(struct graph * g, const char * name);
    void add_edge_graph(struct graph * g, const char * name_from, const char * name_to, double value);
    void print_graph(struct graph * g);
    void reset_graph(struct graph * g);
    
    void init_node_graph(struct node_graph * n,const char * name);
    void init_edge_graph(struct edge_graph * e);
    void init_edge_graph_params(struct edge_graph * e,struct node_graph * to,double value);

    
#ifdef __cplusplus
}
#endif

#endif /* GRAPH_H */

