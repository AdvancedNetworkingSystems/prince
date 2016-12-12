/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "algorithms.h"
#include <limits.h>
//pseudocode from https://en.wikipedia.org/wiki/Tarjan's_strongly_connected_components_algorithm

const int INFINITY=INT_MAX;

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
            printf(" \n");
            (*bcc_id)++;
        }
    }
}


void tarjan_iter(struct graph * g){
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

//http://algo.uni-konstanz.de/publications/b-vspbc-08.pdf
//file:///home/principale/Desktop/res_pro/brandes.png //TODO: remove

void betweeness_brandes(struct graph * g){
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
            if(s->neighbours.size>0){
                // printf("here %s\n",s->name);
                struct node_list * edge_iterator=s->neighbours.head;
                for(edge_iterator=s->neighbours.head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                    struct edge_graph * edge=(struct edge_graph*)edge_iterator->content;
                    struct node_graph * w=edge->to;
                    double weight=edge->value;
                    if(dist[w->id]>(dist[v->id]+weight)){
                        dist[w->id]=dist[v->id]+weight;
                        // printf("here pushing %s\n",w->name);
                        insert_or_update_priority_queue(&q,w,dist[w->id]);
                        sigma[w->id]=0;
                        clear_list(pred +w->id);
                    }
                    // printf("here  not computing %s %f %f\n",w->name,dist[w->id],dist[v->id]+weight);
                    if(dist[w->id]==(dist[v->id]+weight)){
                        //  printf("here computing %s\n",w->name);
                        sigma[w->id]= sigma[w->id]+ sigma[v->id];
                        enqueue_list( pred+w->id,v);
                    }
                }
            }
        }
        printf("HERE %p %d\n",s,S.size);
        //    for( i =0;i<node_num;i++){
        
        //  }
        while(!is_empty_list(&S)){
            struct node_graph * w=(struct node_graph * )pop_list(&S);
            // printf("=========== pop %p %d (%d)\n",s,S.size,pred[w->id].size);
            struct node_list * node_iterator;
            //printf("# %d \n",pred[w->id].size);
            printf("The following nodes has a value, ");
            for(node_iterator =pred[w->id].head;node_iterator!=0;node_iterator=node_iterator->next){
                struct node_graph * v=(struct node_graph*)node_iterator->content;
                delta[v->id]= delta[v->id]+((sigma[v->id]/ sigma[w->id])*(1+delta[w->id]));
                // printf("@> %d %d %f\n",v->id,w->id,delta[v->id]);
                printf("%d, ",v->id);
            }
            printf("\n");
            if(w!=s)
                printf(" We set %d\n",w->id);
            if(w!=s){
                //  printf("%f %f %f=>",ret_val[w->id],delta[w->id],ret_val[w->id]+delta[w->id]);
                //printf("=> %d %f\n",w->id,delta[w->id]);
                ret_val[w->id]=ret_val[w->id]+delta[w->id];
                //printf("§§ %f %f\n",ret_val[w->id],delta[w->id]);
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
}


/*
main(){
    // printf("\nSCCs in zeroth graph \n");
    struct graph g0;
    init_graph(&g0);
    add_edge_graph(&g0,"a","b",0.4);
    
    add_edge_graph(&g0,"b","c",2);
    
    add_edge_graph(&g0,"c","a",1.1);
    
    add_edge_graph(&g0,"d","b",3);
    add_edge_graph(&g0,"d","c",0.7);
    add_edge_graph(&g0,"d","e",1);
    
    add_edge_graph(&g0,"e","d",1.5);
    add_edge_graph(&g0,"e","f",0.1);
    
    add_edge_graph(&g0,"f","c",2.3);
    add_edge_graph(&g0,"f","g",1.8);
    
    add_edge_graph(&g0,"g","f",2.4);
    
    add_edge_graph(&g0,"h","g",2.6);
    add_edge_graph(&g0,"h","e",2.1);
    add_edge_graph(&g0,"h","h",0.5);
    // tarjan_rec(&g0);
    // printf("===============\n");
    // reset_graph(&g0);
    // tarjan_iter(&g0);
    //betweeness_brandes(&g0);
    //exit(0);
    
    //printf("\nSCCs in first graph \n");
    struct graph g1;
    init_graph(&g1);
    add_edge_graph(&g1,"1","0",1);
    add_edge_graph(&g1,"0","2",1);
    add_edge_graph(&g1,"2","1",1);
    add_edge_graph(&g1,"0","3",1);
    add_edge_graph(&g1,"3","4",1);
    //tarjan_rec(&g1);
    //printf("===============\n");
    //reset_graph(&g1);
    //tarjan_iter(&g1);
    betweeness_brandes(&g1);
    
    // printf("\nSCCs in second graph \n");
    struct graph g2;
    init_graph(&g2);
    add_edge_graph(&g2,"0","1",1);
    add_edge_graph(&g2,"1","2",1);
    add_edge_graph(&g2,"2","3",1);
    //tarjan_rec(&g2);
    //printf("===============\n");
    // reset_graph(&g2);
    //tarjan_iter(&g2);
    betweeness_brandes(&g2);
    
    // printf("\nSCCs in third graph \n");
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
    //tarjan_rec(&g3);
    //printf("===============\n");
    //reset_graph(&g3);
    //tarjan_iter(&g3);
    betweeness_brandes(&g3);
    
    //printf("\nSCCs in fourth graph \n");
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
    //tarjan_rec(&g4);
    //printf("===============\n");
    // reset_graph(&g4);
    // tarjan_iter(&g4);
    betweeness_brandes(&g4);
    
    // printf("\nSCCs in fifth graph \n");
    struct graph g5;
    init_graph(&g5);
    add_edge_graph(&g5,"0","1",1);
    add_edge_graph(&g5,"1","2",1);
    add_edge_graph(&g5,"2","3",1);
    add_edge_graph(&g5,"2","4",1);
    add_edge_graph(&g5,"3","0",1);
    add_edge_graph(&g5,"4","2",1);
    //tarjan_rec(&g5);
    //printf("===============\n");
    //reset_graph(&g5);
    //tarjan_iter(&g5);
    betweeness_brandes(&g5);
    return 0;
}*/