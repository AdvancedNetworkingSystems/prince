/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "graph.h"

void init_graph(struct graph * g){
    init_list(&(g->nodes));
}

struct node_graph * add_node_graph(struct graph * g, const char * name){//uniqueness check not performed
    struct node_graph * n=(struct node_graph*)malloc(sizeof(struct node_graph));
    init_node_graph(n,name,g->nodes.size);
    enqueue_list(&(g->nodes),(void*)n);
    return n;
}

void add_edge_graph(struct graph * g, const char * name_from, const char * name_to, double value, bool directed){
    struct node_graph *from=0,*to=0,*current =0;
    struct node_list * n=g->nodes.head;
    while(n!=0 && (from==0 || to==0)){ //if there are no more nodes or we have found both edge ends
        current=(struct node_graph *)n->content;
        if(from==0 && strcmp(current->name,name_from)==0){
            from=current;
        }
        if(to==0 && strcmp(current->name,name_to)==0){
            to=current;
        }
        n=n->next;
    }
    if(from==0){
        from=add_node_graph(g,name_from);
        if(strcmp(name_from,name_to)==0)
            to=from;
    }     
    if(to==0){
        to=add_node_graph(g,name_to);
    }
    if(from!=0 && to!=0){
        struct edge_graph * e=(struct edge_graph*)malloc(sizeof(struct edge_graph));
        init_edge_graph_params(e,to,value);
        enqueue_list(&(from->neighbours),(void*)e);
        if(!directed){ 
            struct edge_graph * e_r=(struct edge_graph*)malloc(sizeof(struct edge_graph));
            init_edge_graph_params(e_r,from,value);
            enqueue_list(&(to->neighbours),(void*)e_r);
        }
    }
}
void add_edge_graph_return_node_indexes(struct graph * g, const char * name_from, const char * name_to, double value, bool directed,int  * nodefrom, int * nodeto){
    struct node_graph *from=0,*to=0,*current =0;
    struct node_list * n=g->nodes.head;
    while(n!=0 && (from==0 || to==0)){ //if there are no more nodes or we have found both edge ends
        current=(struct node_graph *)n->content;
        if(from==0 && strcmp(current->name,name_from)==0){
            from=current;
        }
        if(to==0 && strcmp(current->name,name_to)==0){
            to=current;
        }
        n=n->next;
    }
    if(from==0){
        from=add_node_graph(g,name_from);
        if(strcmp(name_from,name_to)==0)
            to=from;
    }     
    if(to==0){
        to=add_node_graph(g,name_to);
    }
    if(from!=0 && to!=0){
        if(nodefrom!=0)
            (*nodefrom)=from->node_graph_id;
        if(nodeto!=0)
            (*nodeto)=to->node_graph_id;
        struct edge_graph * e=(struct edge_graph*)malloc(sizeof(struct edge_graph));
        init_edge_graph_params(e,to,value);
        enqueue_list(&(from->neighbours),(void*)e);
        if(!directed){ 
            struct edge_graph * e_r=(struct edge_graph*)malloc(sizeof(struct edge_graph));
            init_edge_graph_params(e_r,from,value);
            enqueue_list(&(to->neighbours),(void*)e_r);
        }
    } 
    
}
void print_graph(struct graph * g){
    struct node_list * nq=g->nodes.head;
    while(nq!=0){
        struct node_graph * ng=(struct node_graph*)nq->content;
        struct node_list * nqi=ng->neighbours.head;
        printf("%s (%d) [",ng->name,ng->node_graph_id);
        while(nqi!=0){
            struct edge_graph * eg=(struct edge_graph*)nqi->content;
            printf(" (%s , %f) ",eg->to->name,eg->value);
            nqi=nqi->next;
        } 
        printf("]\n");
        nq=nq->next;
    }
}

void init_node_graph(struct node_graph * n,const char * name,int node_graph_id){
    n->name=name;
    init_list(&(n->neighbours));
   /* n->index=-1;
    n->low_link=-1;
    n->on_stack=false;
    n->bcc_id=-1;*/
    n->node_graph_id=node_graph_id;
    
}

void init_edge_graph(struct edge_graph * e){
    e->to=0;
    e->value=0;
}
void init_edge_graph_params(struct edge_graph * e,struct node_graph * to,double value){
    e->to=to;
    e->value=value;
}

void free_graph(struct graph * g){
    struct node_list * nq=g->nodes.head;
    while(nq!=0){
        struct node_list * nq_tmp=nq;
        struct node_graph * ng=(struct node_graph*)nq->content;
        struct node_list * eq=(struct node_list*)ng->neighbours.head;
        struct node_list * eq_tmp;
        while(eq!=0){
            eq_tmp=eq;
            struct edge_graph *e=(struct edge_graph*)eq->content;
            eq=eq->next;
            free(eq_tmp);
            free(e);
        }
        nq=nq->next;
        free(ng);
        free(nq_tmp);
    }
}