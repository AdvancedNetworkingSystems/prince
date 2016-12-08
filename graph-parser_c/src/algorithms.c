/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "graph/graph.h"

//pseudocode from https://en.wikipedia.org/wiki/Tarjan's_strongly_connected_components_algorithm

void strongconnect(struct node_graph* v,int * index, struct list * s,int * bcc_id);  

inline int min(int a, int b){
    if(a<b)
        return a;
    return b;
}

void tarjan(struct graph * g){
    struct list s;
    init_list(&s);
    
    int index=0;
    int bcc_id=0;
    struct node_list * n =g->nodes.head;
    while(n!=0){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(ng->link<0){
            strongconnect(ng,&index,&s,&bcc_id);
        }
        n=n->next;
    }
    
}


void strongconnect(struct node_graph* v,int * index, struct list * s,int * bcc_id){
    if(v->bcc_id>0)
        return;
    v->index=*index;
    v->low_link=*index;
    (*index)++;
    enqueue_list(s,(void*)v);
    v->on_stack=true;
    
    struct node_list * nq=v->neighbours.head;
    while(nq!=0){
        struct node_graph * w=((struct edge_graph*)nq->content)->to;
        //printf("=> %d %s \n",w->index,w->name);
        if(w->index<0){
            //printf("quiiiiiii\n");
            strongconnect(w,index,s,bcc_id);
            v->low_link=min(v->low_link,w->low_link);
        }else if(w->on_stack){
            v->low_link=min(v->low_link,w->index);
        }
        nq=nq->next;
    }
    if(v->low_link==v->index){
        struct node_graph *w=0;
        do{
            w= (struct node_graph *) pop_list(s);
            w->on_stack=false;
            w->bcc_id=*bcc_id;
            printf("%s ",w->name);
        }while(w!=v);
        printf("\n");
        (*bcc_id)++;
    }
}

main(){
    struct graph g;
    init_graph(&g);
    add_edge_graph(&g,"a","b",1);
    
    add_edge_graph(&g,"b","c",1);
    
    add_edge_graph(&g,"c","a",1);
    
    add_edge_graph(&g,"d","b",1);
    add_edge_graph(&g,"d","c",1);
    add_edge_graph(&g,"d","e",1);
    
    add_edge_graph(&g,"e","d",1);
    add_edge_graph(&g,"e","f",1);
    
    add_edge_graph(&g,"f","c",1);
    add_edge_graph(&g,"f","g",1);
    
    add_edge_graph(&g,"g","f",1);
    
    add_edge_graph(&g,"h","g",1);
    add_edge_graph(&g,"h","e",1);
    add_edge_graph(&g,"h","h",1);
    // print_graph(&g);
    printf("===============\n");
    tarjan(&g);
}