/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "algorithms.h"
#include <limits.h>
//pseudocode from https://en.wikipedia.org/wiki/Tarjan's_strongly_connected_components_algorithm

const int INFINITY=INT_MAX;

void strongconnect(struct node_graph* v,int * index, struct list * s,int * bcc_id,struct list* connected_components);  
void DFS_iter(struct node_graph* u,int * index,struct list * s,int *bcc_id,struct list* connected_components);

inline int min(int a, int b){
    if(a<b)
        return a;
    return b;
}

struct list*  tarjan_rec(struct graph * g){
    struct list * connected_components=( struct list * )malloc(sizeof( struct list ));
    init_list(connected_components);
    struct list s;
    init_list(&s);
    
    int index=0;
    int bcc_id=0;
    struct node_list * n =g->nodes.head;
    
    while(n!=0){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(ng->index<0){
            strongconnect(ng,&index,&s,&bcc_id,connected_components);
        }
        n=n->next;
    }
    return connected_components;
}

void strongconnect(struct node_graph* v,int * index, struct list * s,int * bcc_id,struct list* connected_components){
    v->index=*index;
    v->low_link=*index;
    (*index)++;
    enqueue_list(s,(void*)v);
    v->on_stack=true;
    struct node_list * nq=v->neighbours.head;
    while(nq!=0){
        struct node_graph * w=((struct edge_graph*)nq->content)->to;
        if(w->index<0){
            strongconnect(w,index,s,bcc_id,connected_components);
            v->low_link=min(v->low_link,w->low_link);
        }else if(w->on_stack){
            v->low_link=min(v->low_link,w->index);
        }
        nq=nq->next;
    }
    
    if(v->low_link==v->index){
        struct node_graph *w=0;
        struct list * cc_list=( struct list * )malloc(sizeof( struct list ));
        init_list(cc_list);
        do{
            w= (struct node_graph *) pop_list(s);
            w->on_stack=false;
            if(w->bcc_id<0){//To avoid renaming
                enqueue_list(cc_list,w);
                w->bcc_id=*bcc_id;
            }
        }while(w!=v);
        if(cc_list->size>0){
            struct connected_component * connected=( struct connected_component * )malloc(sizeof( struct connected_component ));
            connected->nodes=cc_list;
            connected->id=*bcc_id;
            enqueue_list(connected_components,connected);
            (*bcc_id)++;
        }else{
            free(cc_list);
        }
    }
}


struct list*  tarjan_iter(struct graph * g){
    struct list * connected_components=( struct list * )malloc(sizeof( struct list ));
    init_list(connected_components);
    struct node_list * n =g->nodes.head;
    int index=0;
    int bcc_id=0;
    struct list s;
    init_list(&s);
    while(n!=0){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(ng->index<0){
            DFS_iter(ng,&index,&s,&bcc_id,connected_components);
        }
        n=n->next;
    }
    return connected_components;
}

//https://www.researchgate.net/profile/Oscar_Karnalim/publication/303959022_Improving_Scalability_of_Java_Archive_Search_Engine_through_Recursion_Conversion_And_Multithreading/links/57c929ed08aefc4af350b37d.pdf?origin=publication_detail
//http://stackoverflow.com/questions/2292223/iterative-version-of-a-recursive-algorithm-is-slower
void DFS_iter(struct node_graph* u,int * index,struct list * s,int *bcc_id,struct list* connected_components){
    u->index=*index;
    u->low_link=*index;
    (*index)++;
    u->iterator=u->neighbours.head;
    u->caller=0;
    u->on_stack=true,
    enqueue_list(s,(void*)u);
    struct node_graph* last=u;
    while(true){
        if(last->iterator!=0){
            struct node_graph *w = ((struct edge_graph*)last->iterator->content)->to;
            last->iterator=last->iterator->next; 
            if(w->index<0){
                w->caller=last;
                w->index=*index;
                w->low_link=*index;
                (*index)++;
                enqueue_list(s,(void*)w);
                w->on_stack=true;
                w->iterator=w->neighbours.head;
                last=w;
            }else if(w->on_stack){
                last->low_link=min(last->low_link,w->index);
            }
        }else{
            if(last->index==last->low_link){
                struct list * cc_list=( struct list * )malloc(sizeof( struct list ));
                init_list(cc_list);
                struct node_graph* top=0;
                do{
                    top =(struct node_graph*)pop_list(s);
                    if(top!=0){
                        enqueue_list(cc_list,top);
                        top->on_stack=false;                    
                    }
                }while(top!=0 && top->index!=last->index);
                if(cc_list->size>0){
                    struct connected_component * connected=( struct connected_component * )malloc(sizeof( struct connected_component ));
                    connected->nodes=cc_list;
                    connected->id=*bcc_id;
                    enqueue_list(connected_components,connected);
                    (*bcc_id)++;
                }else{
                    free(cc_list);
                }
                
            }
            struct node_graph *new_last=last->caller;
            if(new_last!=0){
                new_last->low_link=min(new_last->low_link,last->low_link);
                last=new_last;
            }else{
                break;
            }
        }
    }
}

//http://algo.uni-konstanz.de/publications/b-vspbc-08.pdf

double * betweeness_brandes(struct graph * g){
    struct priority_queue q;
    struct list S;
    init_priority_queue(&q);
    init_priority_queue(&q);
    init_list(&S);
    int node_num=g->nodes.size;
    double * dist=malloc(node_num*sizeof(double));
    struct list * pred=malloc(node_num*sizeof( struct list));
    int * sigma=malloc(node_num*sizeof(int));
    double * delta=malloc(node_num*sizeof(double));
    double * ret_val=malloc(node_num*sizeof(double));
    int i;
    for( i =0;i<node_num;i++){
        ret_val[i]=0;
        init_list(pred +i);
    }
    struct node_list * n=0;
    
    for(n=g->nodes.head;n!=0; n=n->next){
        struct node_graph* s=(struct node_graph*) n->content;
        for( i =0;i<node_num;i++){
            clear_list(pred +i);
            struct list * tmp=pred +i;
            dist[i]=INFINITY;
            sigma[i]=0;
            delta[i]=0;
        }
        dist[s->id]=0;
        sigma[s->id]=1;
        insert_priority_queue(&q,(void*)s,0);
        
        while(!is_empty_priority_queue(&q)){
            struct node_graph* v=(struct node_graph*)dequeue_priority_queue(&q);
            enqueue_list(&S,v);
            if(v->neighbours.size>0){
                struct node_list * edge_iterator;
                for(edge_iterator=v->neighbours.head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                    struct edge_graph * edge=(struct edge_graph*)edge_iterator->content;
                    struct node_graph * w=edge->to;
                    double weight=edge->value;
                    if(dist[w->id]>(dist[v->id]+weight)){
                        dist[w->id]=dist[v->id]+weight;
                        insert_or_update_priority_queue(&q,w,dist[w->id]);
                        sigma[w->id]=0;
                        clear_list(pred +w->id);
                    }if(dist[w->id]==(dist[v->id]+weight)){
                        sigma[w->id]= sigma[w->id]+ sigma[v->id];
                        enqueue_list( pred+w->id,v);
                    }
                }
            }
        }
        if(false){//TODO: remove
            for( i =0;i<node_num;i++){
                struct list * tmp=pred+i;
                struct node_list * n_t;;
                printf("%s: [",s->name);
                for(n_t=tmp->head;n_t!=0;n_t=n_t->next){
                    struct node_graph * w=(struct node_graph *)n_t->content;
                    printf("%s ,",w->name);
                }
                printf("],");
            }
            printf("\n");
        }
        while(!is_empty_list(&S)){
            struct node_graph * w=(struct node_graph * )pop_list(&S);
            struct node_list * node_iterator;
            for(node_iterator =pred[w->id].head;node_iterator!=0;node_iterator=node_iterator->next){
                struct node_graph * v=(struct node_graph*)node_iterator->content;
                delta[v->id]= delta[v->id]+((((double)sigma[v->id])/ ((double)sigma[w->id]))*(1+delta[w->id]));
            }
            if(w!=s){
                ret_val[w->id]=ret_val[w->id]+delta[w->id];
            }
        }
    }
    for(n=g->nodes.head;n!=0; n=n->next){
        printf("%s : %f\n",((struct node_graph*)n->content)->name,ret_val[((struct node_graph*)n->content)->id]);
    }
    free(dist);
    free(pred);
    free(sigma);
    free(delta);
    return ret_val;
}

void main(){
    struct graph g;
    init_graph(&g);
    add_edge_graph(&g,"0","2",8);
    add_edge_graph(&g,"1","0",7);
    add_edge_graph(&g,"1","2",3);
    add_edge_graph(&g,"1","3",3);
    add_edge_graph(&g,"2","0",7);
    add_edge_graph(&g,"2","1",6);
    add_edge_graph(&g,"2","3",10);
    add_edge_graph(&g,"2","4",1);
    add_edge_graph(&g,"3","0",4);
    add_edge_graph(&g,"3","1",2);
    add_edge_graph(&g,"3","2",9);
    add_edge_graph(&g,"3","4",1);
    add_edge_graph(&g,"4","0",10);
    add_edge_graph(&g,"4","1",5);
    add_edge_graph(&g,"4","2",8);
    betweeness_brandes(&g);
    //0: 0.08333333333333333, 1: 0.25, 2: 0.25, 3: 0.125, 4: 0.041666666666666664
}
