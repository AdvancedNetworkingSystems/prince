/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "graph/graph.h"

//pseudocode from https://en.wikipedia.org/wiki/Tarjan's_strongly_connected_components_algorithm

void strongconnect(struct node_graph* v,int * index, struct list * s,int * bcc_id);  
void DFS_iter(struct node_graph* i,int * index,struct list * s);
inline int min(int a, int b){
    if(a<b)
        return a;
    return b;
}

void tarjan_rec(struct graph * g){
    struct list s;
    init_list(&s);
    
    int index=0;
    int bcc_id=0;
    struct node_list * n =g->nodes.head;
    
    while(n!=0){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(ng->index<0){
            strongconnect(ng,&index,&s,&bcc_id);
        }
        n=n->next;
    }
    
}

void strongconnect(struct node_graph* v,int * index, struct list * s,int * bcc_id){
    v->index=*index;
    v->low_link=*index;
    (*index)++;
    enqueue_list(s,(void*)v);
    v->on_stack=true;
    struct node_list * nq=v->neighbours.head;
    while(nq!=0){
        struct node_graph * w=((struct edge_graph*)nq->content)->to;
        if(w->index<0){
            strongconnect(w,index,s,bcc_id);
            v->low_link=min(v->low_link,w->low_link);
        }else if(w->on_stack){
            v->low_link=min(v->low_link,w->index);
        }
        nq=nq->next;
    }
    
    if(v->low_link==v->index){
        struct node_graph *w=0;
        bool empty=true;
        do{
            w= (struct node_graph *) pop_list(s);
            w->on_stack=false;
            if(w->bcc_id<0){//To avoid renaming
                empty=false;
                w->bcc_id=*bcc_id;
                printf("%s ",w->name);
            }
        }while(w!=v);
        if(!empty){
            printf(" \n",(*bcc_id));
            (*bcc_id)++;
        }
    }
}


tarjan_iter(struct graph * g){
    struct node_list * n =g->nodes.head;
    int index=0;
    struct list s;
    init_list(&s);
    while(n!=0){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(ng->index<0){
            DFS_iter(ng,&index,&s);
        }
        n=n->next;
    }
}

//https://www.researchgate.net/profile/Oscar_Karnalim/publication/303959022_Improving_Scalability_of_Java_Archive_Search_Engine_through_Recursion_Conversion_And_Multithreading/links/57c929ed08aefc4af350b37d.pdf?origin=publication_detail
//http://stackoverflow.com/questions/2292223/iterative-version-of-a-recursive-algorithm-is-slower
void DFS_iter(struct node_graph* u,int * index,struct list * s){
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
                struct node_graph* top=0;
                bool printed=false;
                do{
                    top =(struct node_graph*)pop_list(s);
                    if(top!=0){
                        printed=true;
                        top->on_stack=false;
                        printf("%s ",top->name);                      
                    }
                }while(top!=0 && top->index!=last->index);
                if(printed)
                    printf("\n");
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



main(){
     printf("\nSCCs in zeroth graph \n");
    struct graph g0;
    init_graph(&g0);
    add_edge_graph(&g0,"a","b",1);
    
    add_edge_graph(&g0,"b","c",1);
    
    add_edge_graph(&g0,"c","a",1);
    
    add_edge_graph(&g0,"d","b",1);
    add_edge_graph(&g0,"d","c",1);
    add_edge_graph(&g0,"d","e",1);
    
    add_edge_graph(&g0,"e","d",1);
    add_edge_graph(&g0,"e","f",1);
    
    add_edge_graph(&g0,"f","c",1);
    add_edge_graph(&g0,"f","g",1);
    
    add_edge_graph(&g0,"g","f",1);
    
    add_edge_graph(&g0,"h","g",1);
    add_edge_graph(&g0,"h","e",1);
    add_edge_graph(&g0,"h","h",1);
    tarjan_rec(&g0);
    printf("===============\n");
    reset_graph(&g0);
    tarjan_iter(&g0);
    
    printf("\nSCCs in first graph \n");
    struct graph g1;
    init_graph(&g1);
    add_edge_graph(&g1,"1","0",1);
    add_edge_graph(&g1,"0","2",1);
    add_edge_graph(&g1,"2","1",1);
    add_edge_graph(&g1,"0","3",1);
    add_edge_graph(&g1,"3","4",1);
    tarjan_rec(&g1);
    printf("===============\n");
    reset_graph(&g1);
    tarjan_iter(&g1);
    
    printf("\nSCCs in second graph \n");
    struct graph g2;
    init_graph(&g2);
    add_edge_graph(&g2,"0","1",1);
    add_edge_graph(&g2,"1","2",1);
    add_edge_graph(&g2,"2","3",1);
    tarjan_rec(&g2);
    printf("===============\n");
    reset_graph(&g2);
    tarjan_iter(&g2);
    
    printf("\nSCCs in third graph \n");
    struct graph g3;
    init_graph(&g3);
    add_edge_graph(&g3,"0","1",1);
    add_edge_graph(&g3,"1","2",1);
    add_edge_graph(&g3,"2","0",1);
    add_edge_graph(&g3,"1","3",1);
    add_edge_graph(&g3,"1","4",1);
    add_edge_graph(&g3,"1","6",1);
    add_edge_graph(&g3,"3","5",1);
    add_edge_graph(&g3,"4","5",1);
    tarjan_rec(&g3);
    printf("===============\n");
    reset_graph(&g3);
    tarjan_iter(&g3);
    
    printf("\nSCCs in fourth graph \n");
    struct graph g4;
    init_graph(&g4);
    add_edge_graph(&g4,"0","1",1); 
    add_edge_graph(&g4,"0","3",1);
    add_edge_graph(&g4,"1","2",1); 
    add_edge_graph(&g4,"1","4",1);
    add_edge_graph(&g4,"2","0",1); 
    add_edge_graph(&g4,"2","6",1);
    add_edge_graph(&g4,"3","2",1);
    add_edge_graph(&g4,"4","5",1); 
    add_edge_graph(&g4,"4","6",1);
    add_edge_graph(&g4,"5","6",1); 
    add_edge_graph(&g4,"5","7",1); 
    add_edge_graph(&g4,"5","8",1); 
    add_edge_graph(&g4,"5","9",1);
    add_edge_graph(&g4,"6","4",1);
    add_edge_graph(&g4,"7","9",1);
    add_edge_graph(&g4,"8","9",1);
    add_edge_graph(&g4,"9","8",1);
    tarjan_rec(&g4);
    printf("===============\n");
    reset_graph(&g4);
    tarjan_iter(&g4);
    
    printf("\nSCCs in fifth graph \n");
    struct graph g5;
    init_graph(&g5);
    add_edge_graph(&g5,"0","1",1);
    add_edge_graph(&g5,"1","2",1);
    add_edge_graph(&g5,"2","3",1);
    add_edge_graph(&g5,"2","4",1);
    add_edge_graph(&g5,"3","0",1);
    add_edge_graph(&g5,"4","2",1);
    tarjan_rec(&g5);
    printf("===============\n");
    reset_graph(&g5);
    tarjan_iter(&g5);
    
    return 0;
}