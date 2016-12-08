/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "list.h"

void init_list( struct list * q){
    q->head=0;
    q->tail=0;
    q->size=0;
}
void enqueue_list(struct list * q,void * item){
    struct  node_list * n=(struct node_list*)malloc(sizeof(struct node_list));
    n->content=item;
    if(q->head==0){
        n->prev=0;
        n->next=0;
        q->head=n;
        q->size=1;
        q->tail=n;
    }else{
        n->prev=q->tail;
        q->tail->next=n;
        n->next=0;
        q->tail=n;
        q->size++;
    }
}
void * dequeue_list(struct list * q){
    void * ret_val=0;
    if(q->head!=0){
        struct node_list * to_remove=q->head;
        ret_val=q->head->content;
        if(q->head->next==0){
            q->head=0;
            q->tail=0;
        }else{
            q->head=q->head->next;
            q->head->prev=0;
        }
        free(to_remove);
        q->size--;
    }
    
    return ret_val;
}

void * pop_list(struct list * q){
    void * ret_val=0;
    if(q->tail!=0){
        struct node_list * to_remove=q->tail;
        ret_val=to_remove->content;
        q->tail=to_remove->prev;
        if(q->tail!=0){
            q->tail->next=0;
        }else{
            q->head=0;
        }
        free(to_remove);
        q->size--; 
    }
    return ret_val;
}
void print_list(struct list * q){
    struct node_list * n=q->head;
    while(n!=0){
        printf("%p ",n);
        n=n->next;
    }
    printf("\n");
}

int is_empty_list(struct list * q){
    return q->head==0;
}

void init_priority_queue(struct priority_queue * q){
    q->head=0;
    q->tail=0;
    q->size=0;
}
void enqueue_priority_queue(struct priority_queue * q,void * item, double val){
    struct  node_priority_queue * n=(struct node_priority_queue*)malloc(sizeof(struct node_priority_queue));
    n->content=item;
    n->value=val;
    if(q->head==0){ //if priority list is empty
        n->prev=0;
        n->next=0;
        q->head=n;
        q->size=1;
        q->tail=n;
    }else{
        struct node_priority_queue * n_current=q->head;
        while(n_current!=0&&n_current->value<val){//find the node we have to replace
            n_current=n_current->next;
        }
        if(n_current==0){//if it is the last, we put the node as last
            n->prev=q->tail;
            q->tail->next=n;
            n->next=0;
            q->tail=n;
        }else{//if it is not the last
            if(n_current->prev!=0){//if it is not the first
                n_current->prev->next=n;
                n->prev=n_current->prev;
                n_current->prev=n;
                n->next=n_current;
            }else{ //if it is the first
                q->head->prev=n;
                q->head=n;
                n->prev=0;
                n->next=n_current;
            }
            
        }
        q->size++;
    }
}
void * dequeue_priority_queue(struct priority_queue * q){
    void * ret_val=0;
    if(q->head!=0){
        struct node_priority_queue * to_remove=q->head;
        ret_val=q->head->content;
        if(q->head->next==0){
            q->head=0;
            q->tail=0;
        }else{
            q->head=q->head->next;
            q->head->prev=0;
        }
        q->size--;
        free(to_remove);
    }
    
    return ret_val;
}
void print_priority_queue(struct priority_queue * q){
    struct node_priority_queue * n=q->head;
    while(n!=0){
        printf("%f ",n->value);
        n=n->next;
    } 
    printf("\n");
}

int is_empty_priority_queue(struct priority_queue * q){
    return q->head==0;
}
