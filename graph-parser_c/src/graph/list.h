/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   queue.h
 * Author: principale
 *
 * Created on December 6, 2016, 6:41 PM
 */


#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
    
    
    struct node_queue {
        void * content;
        struct node_queue * next;
        struct node_queue * prev;
    };
    
    
    struct queue {
        struct node_queue * head;
        struct node_queue * tail;
        int size;
    };
    
    struct node_priority_queue {
        void * content;
        struct node_priority_queue * next;
        struct node_priority_queue * prev;
        double value;
    };
    
    struct priority_queue {
        struct node_priority_queue * head;
        struct node_priority_queue * tail;
        int size;
    };
    
    void init_queue(struct queue * q);
    void enqueue_queue(struct queue * q,void * item);
    void * dequeue_queue(struct queue * q);
    void print_queue(struct queue * q);
    int is_empty_queue(struct queue * q);
    
    void init_priority_queue(struct priority_queue * q);
    void enqueue_priority_queue(struct priority_queue * q,void * item, double val);
    void * dequeue_priority_queue(struct priority_queue * q);
    void print_priority_queue(struct priority_queue * q);
    int is_empty_priority_queue(struct priority_queue * q);
    
    
#ifdef __cplusplus
}
#endif

#endif /* LIST_H */

