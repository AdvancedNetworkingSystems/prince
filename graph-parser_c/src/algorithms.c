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
void DFS_iter(struct node_graph* u,int * index,struct list * s,int *bcc_id,
        struct list* connected_components,struct node_graph ** caller, struct node_list ** iterator);
void DFS_visit(struct node_graph * u,struct list *s,int * d,int * low,bool * visited,struct node_graph ** parent,int * count, bool * added, bool * is_articulation_point,int node_num,struct list * connected_components);

inline int min(int a, int b){
    if(a<b)
        return a;
    return b;
}

struct list*  tarjan_rec_dir(struct graph * g){
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
    for(nq=v->neighbours.head;nq!=0; nq=nq->next){
        struct node_graph * w=((struct edge_graph*)nq->content)->to;
        if(w->index<0){
            strongconnect(w,index,s,bcc_id,connected_components);
            v->low_link=min(v->low_link,w->low_link);
        }else if(w->on_stack){
            v->low_link=min(v->low_link,w->index);
        }
        
    }
    
    if(v->low_link==v->index){
        struct node_graph *w=0;
        struct list * cc_list=( struct list * )malloc(sizeof( struct list ));
        init_list(cc_list);
        do{
            w= (struct node_graph *) pop_list(s);
            w->on_stack=false;
            // printf("%s ",w->name);
            //if(w->bcc_id<0){//To avoid renaming
            enqueue_list(cc_list,w);
            w->bcc_id=*bcc_id;
            // }
        }while(w!=v);
        //printf("\n"); 
        if(cc_list->size>0){//TODO: fix for components
            struct connected_component * connected=( struct connected_component * )malloc(sizeof( struct connected_component ));
            //    connected->nodes=cc_list;
            //    connected->id=*bcc_id;
            enqueue_list(connected_components,connected);
            (*bcc_id)++;
        }else{
            free(cc_list);
        }
    }
}


struct list*  tarjan_iter_dir(struct graph * g){
    struct list * connected_components=( struct list * )malloc(sizeof( struct list ));
    init_list(connected_components);
    struct node_list * n =g->nodes.head;
    int index=0;
    int bcc_id=0;
    struct list s;
    init_list(&s);
    struct node_graph ** caller=(struct node_graph **)malloc(sizeof(struct node_graph *)*g->nodes.size);
    struct node_list ** iterator=(struct node_list **)malloc(sizeof(struct node_list *)*g->nodes.size);
    while(n!=0){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(ng->index<0){
            DFS_iter(ng,&index,&s,&bcc_id,connected_components,caller,iterator);
        }
        n=n->next;
    }
    free(caller);
    free(iterator);
    return connected_components;
}

//https://www.researchgate.net/profile/Oscar_Karnalim/publication/303959022_Improving_Scalability_of_Java_Archive_Search_Engine_through_Recursion_Conversion_And_Multithreading/links/57c929ed08aefc4af350b37d.pdf?origin=publication_detail
//http://stackoverflow.com/questions/2292223/iterative-version-of-a-recursive-algorithm-is-slower
void DFS_iter(struct node_graph* u,int * index,struct list * s,int *bcc_id,
        struct list* connected_components,struct node_graph ** caller, struct node_list ** iterator){
    u->index=*index;
    u->low_link=*index;
    (*index)++;
    iterator[u->id]=u->neighbours.head;
    //u->iterator=u->neighbours.head;
    caller[u->id]=0;
    // u->caller=0;
    u->on_stack=true,
    enqueue_list(s,(void*)u);
    struct node_graph* last=u;
    while(true){
        // if(last->iterator!=0){
        if(iterator[last->id]!=0){
            // struct node_graph *w = ((struct edge_graph*)last->iterator->content)->to;
            //last->iterator=last->iterator->next;
            struct node_graph *w = ((struct edge_graph*)iterator[last->id]->content)->to;
            iterator[last->id]=iterator[last->id]->next; 
            if(w->index<0){
                caller[w->id]=last;
                //w->caller=last;
                w->index=*index;
                w->low_link=*index;
                (*index)++;
                enqueue_list(s,(void*)w);
                w->on_stack=true;
                // w->iterator=w->neighbours.head;
                iterator[w->id]=w->neighbours.head;
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
                if(cc_list->size>0){//TODO: fix for components
                    struct connected_component * connected=( struct connected_component * )malloc(sizeof( struct connected_component ));
                    //                    connected->nodes=cc_list;
                    //      connected->id=*bcc_id;
                    enqueue_list(connected_components,connected);
                    (*bcc_id)++;
                }else{
                    free(cc_list);
                }
                
            }
            // struct node_graph *new_last=last->caller;
            struct node_graph *new_last=caller[last->id];
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
    free(dist);
    free(pred);
    free(sigma);
    free(delta);
    return ret_val;
}
//From http://www.cs.umd.edu/class/fall2005/cmsc451/biconcomps.pdf
struct list*  tarjan_rec_undir(struct graph * g, bool * is_articulation_point){
    struct list * connected_components=( struct list * )malloc(sizeof( struct list ));
    init_list(connected_components);
    int node_num=g->nodes.size;
    int count=0;
    bool * visited=malloc(node_num*sizeof(bool));
    bool * added=malloc(node_num*sizeof(bool));
    struct node_graph ** parent=(struct node_graph **)malloc(sizeof(struct node_graph *)*g->nodes.size);
    struct list s;
    init_list(&s);
    int * d=malloc(node_num*sizeof(int));
    int * low=malloc(node_num*sizeof(int));
    int i;
    for(i=0;i<node_num;i++){
        visited[i]=false;
        added[i]=false;
        parent[i]=0;
        d[i]=-1;
        low[i]=-1;
        is_articulation_point[i]=false;
    }
    struct node_list * n;
    i=0;
    for(n =g->nodes.head;n!=0;n=n->next){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(!visited[i]){
            DFS_visit(ng,&s,d,low,visited,parent,&count,added, is_articulation_point,node_num,connected_components);
        }
        i++;
    }
    free(added);
    free(visited);
    free(parent);
    free(d);
    free(low);
    return connected_components;
}

struct edge_repr{
    struct node_graph *from;
    struct node_graph *to;  
    double value;
};
struct edge_repr * init_edge_repr(struct node_graph * from,struct node_graph * to, double value){
    struct edge_repr * er=(struct edge_repr * )malloc(sizeof(struct edge_repr ));
    er->from=from;
    er->to=to;
    er->value=value;
    return er;
}

//https://www.cs.umd.edu/class/fall2005/cmsc451/biconcomps.pdf
void DFS_visit(struct node_graph * u,struct list * s,int * d,int * low,bool * visited,
        struct node_graph ** parent,int * count, bool * added, bool * is_articulation_point,int node_num,struct list * connected_components){
    visited[u->id]=true;
    d[u->id]=(*count);
    low[u->id]=(*count);
    (*count)=(*count)+1;
    if(u->neighbours.size>0){
        struct node_list * edge_iterator;
        for(edge_iterator=u->neighbours.head;edge_iterator!=0;edge_iterator=edge_iterator->next){
            struct edge_graph * edge=(struct edge_graph*)edge_iterator->content;
            struct node_graph * v=edge->to;
            if(!visited[v->id]){
                visited[v->id]=true;
                /*   struct edge_repr * er=(struct edge_repr * )malloc(sizeof(struct edge_repr ));
                 er->from=u;
                 er->to=v;
                 er->value=edge->value;*/
                struct edge_repr * er=init_edge_repr(u,v,edge->value);
                enqueue_list(s,er);
                parent[v->id]=u;
                DFS_visit(v,s,d,low,visited,parent,count,added,is_articulation_point,node_num,connected_components);
                
                if(low[v->id]>=d[u->id]){
                    struct edge_repr * er_i=0;
                    struct list l;
                    init_list(&l);
                    do{
                        er_i=( struct edge_repr *)pop_list(s);
                        added[er_i->from->id]=true;
                        added[er_i->to->id]=true;
                        enqueue_list(&l,er_i);
                    }  while(!is_empty_list(s) && !(er_i->to==er->to && er_i->from==er->from ));
                    
                    int added_count=0;
                    int i;
                    for(i=0;i<node_num;i++){
                        if(added[i]){
                            added_count++;
                            added[i]=false;
                        }
                    }
                    if(!is_empty_list(&l)){
                        struct connected_component * cc=(struct connected_component *)malloc(sizeof(struct connected_component));
                        init_graph(&(cc->g));
                        cc->mapping=(int *)malloc(sizeof(int)*added_count);
                        cc->map_size=added_count;
                        cc->cutpoint=0;
                        while(!is_empty_list(&l)){
                            struct edge_repr * er_i=( struct edge_repr *)pop_list(&l);
                            int f=0,t=0;
                            add_edge_graph_return_node_indexes(&(cc->g), er_i->from->name, er_i->to->name, er_i->value,0,&f, &t);
                            cc->mapping[f]= er_i->from->id;
                            cc->mapping[t]= er_i->to->id;
                            free(er_i);
                        }
                        enqueue_list(connected_components,cc);
                    }
                    clear_list(&l);
                }
                if((u->neighbours.size>1)&&(parent[u->id]!=0 && low[v->id] >= d[u->id])){
                    is_articulation_point[u->id]=true;
                }
                low[u->id]=min(low[u->id],low[v->id]);
                
            }else if((parent[u->id]!=v)&&(d[v->id]<d[u->id])){
                /*struct edge_repr * er=(struct edge_repr * )malloc(sizeof(struct edge_repr ));
                 er->from=u;
                 er->to=v;
                 er->value=edge->value;*/
                struct edge_repr * er=init_edge_repr(u,v,edge->value);
                enqueue_list(s,er);
                low[u->id]=min(low[u->id],d[v->id]);
            }
        }
    }
}


//https://www.google.it/url?sa=t&rct=j&q=&esrc=s&source=web&cd=10&cad=rja&uact=8&ved=0ahUKEwiqxLjNo_nQAhWCPxQKHSwyAUkQFghkMAk&url=https%3A%2F%2Fwww.cs.duke.edu%2Fcourses%2Ffall05%2Fcps130%2Flectures%2Freif.lectures%2FALG5.1.ppt&usg=AFQjCNHYopbww378Ke-abSmSNdIoWGAP8w&sig2=ygNiUqK_CQIcYJK6phyp0g
struct list*  tarjan_iter_undir(struct graph * g){
    
}

// normal defines whether the pair is (B,v) (if false, (v,B)). Since ordering 
// would be a major computational effort, a boolean ("normal") indicates that.
struct cc_node_edge{
    struct connected_component * from;
    struct node_graph * to;
    int index;
    bool normal;
};

struct cc_node_edge * init_cc_node_edge(struct connected_component * from,struct node_graph * to,int index,bool normal){
    struct cc_node_edge * cne=(struct cc_node_edge * )malloc(sizeof(struct cc_node_edge ));
    cne->from=from;
    cne->to=to;
    cne->index=index;
    cne->normal=normal;
    return cne;
}

struct cc_node_edge * clone_cc_node_edge(struct cc_node_edge *c){
    struct cc_node_edge * cne=(struct cc_node_edge * )malloc(sizeof(struct cc_node_edge ));
    cne->from=c->from;
    cne->to=c->to;
    cne->index=c->index;
    cne->normal=c->normal;
    return cne;
}

struct list*  connected_components_to_tree(struct graph * g, struct list* connected_components, bool * is_articulation_point){
    struct list* tree_edges=(struct list*)malloc(sizeof(struct list));
    init_list(tree_edges);
    struct node_list * ccs_iterator;
    bool * visited=malloc(sizeof(bool)*g->nodes.size);
    struct node_graph ** nodes=malloc(sizeof( struct node_graph *)*g->nodes.size);
    int i;
    struct node_list * node_iterator=g->nodes.head;
    for(i=0;i<g->nodes.size;i++){
        visited[i]=false;
        nodes[i]=(struct node_graph *)node_iterator->content;
        node_iterator=node_iterator->next;
    }
    int cc_edge_counter=0;
    for(ccs_iterator=connected_components->head;ccs_iterator!=0;ccs_iterator=ccs_iterator->next){
        struct connected_component * cc= ( struct connected_component *)ccs_iterator->content;
        for(i=0;i<cc->map_size;i++){
            visited[cc->mapping[i]]=true;
        }
        int art_points=0;
        struct node_graph * ng=0;
        for(i=0;i<cc->map_size;i++){
            int new_index=cc->mapping[i];
            if(is_articulation_point[new_index]){
                art_points++;
                ng=nodes[new_index];
                struct node_graph * n=nodes[new_index];
                n->neighbours.head;
                struct node_list * edge_iterator;
                
                for(edge_iterator=n->neighbours.head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                    struct edge_graph * edge=(struct edge_graph*)edge_iterator->content;
                    //TODO: check if we have to add both CC, outgoing and incoming
                    if(!visited[edge->to->id]){
                        struct cc_node_edge * cne=init_cc_node_edge(cc,edge->to,cc_edge_counter++,true);
                        enqueue_list(tree_edges,cne);
                    }
                }
                
                //  printf("%s\n",nodes[new_index]->name);
            }
        }
        for(i=0;i<cc->map_size;i++){
            visited[cc->mapping[i]]=false;
        }
        if(art_points==1){
            cc->cutpoint=ng;
        }
    }
    free(visited);
    free(nodes);
    return tree_edges;
}

//From http://algo.uni-konstanz.de/publications/pzedb-hsbcc-12.pdf
double * compute_component_tree_weights(struct graph * g, struct list* tree_edges, struct list* connected_components){
    struct list q;
    init_list(&q);
    int v_num=g->nodes.size;
    
    /*
     struct node_list * ccs_iterator;
     for(ccs_iterator=connected_components->head;ccs_iterator!=0;ccs_iterator=ccs_iterator->next){
     struct connected_component * cc= ( struct connected_component *)ccs_iterator->content;
     if(cc->cutpoint!=0){
     struct cc_node_edge *twp=cc_node_edge(cc,cc->cutpoint,true);
     enqueue_list(&q,twp);
     } 
     }*/
    struct node_list * edge_iterator;
    int edges_num=tree_edges->size;
    for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
        struct cc_node_edge * cne=(struct cc_node_edge *)edge_iterator->content;
        if(cne->from->cutpoint!=0){
            struct cc_node_edge * cne_i=init_cc_node_edge( cne->from,cne->from->cutpoint,edges_num++,true);//Which index
            enqueue_list(&q,cne_i);
        }
    }
    double * tree_edges_weights=malloc(sizeof(double)*edges_num);
    int i;
    for(i=0;i<edges_num;i++){
        tree_edges_weights[i]=-1;
    }
    
    while(!is_empty_list(&q)){
        struct cc_node_edge * cne=dequeue_list(&q);
        if(cne->normal){
            int node_num=cne->from->map_size;
            int size=node_num-1;
            struct node_list * edge_iterator;
            for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                struct cc_node_edge * cne_i=(struct cc_node_edge*)edge_iterator->content;
                if(cne_i!=cne&&cne_i->from==cne->from&&tree_edges_weights[cne_i->index]!=-1){
                    size+=v_num-tree_edges_weights[cne_i->index];
                }
            }
            tree_edges_weights[cne->index]=size;
            if(tree_edges_weights[cne->index]<0){
                printf("S1 %f\n",tree_edges_weights[cne->index]);
            }
            for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                struct cc_node_edge * cne_i=(struct cc_node_edge*)edge_iterator->content;
                //cne_i->from!=cne->from&&
                // printf("2. %p %p %d\n",cne_i->to,cne->to,cne_i->to==cne->to);
                if(cne_i->to==cne->to&&(tree_edges_weights[cne_i->index]==-1)){
                    //  cne_i=clone_cc_node_edge(cne_i);
                    cne_i->normal=false;
                    enqueue_list(&q,cne_i);
                }
            }
        }else{
            // printf("qui2\n");
            int size=1;
            struct node_list * edge_iterator;
            for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                struct cc_node_edge * cne_i=(struct cc_node_edge*)edge_iterator->content;
                if(cne_i!=cne&&cne_i->to==cne->to&&tree_edges_weights[cne_i->index]!=-1){
                    size+=tree_edges_weights[cne_i->index];
                }
            }
            tree_edges_weights[cne->index]=v_num-1-size;
            if(tree_edges_weights[cne->index]<0){
                printf("S2 %f %d %d\n",tree_edges_weights[cne->index],v_num,size);
            }
            for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                struct cc_node_edge * cne_i=(struct cc_node_edge*)edge_iterator->content;
                if(cne_i->from==cne->from&&(tree_edges_weights[cne_i->index]==-1)){
                    //cne_i=clone_cc_node_edge(cne_i);
                    cne_i->normal=true;
                    enqueue_list(&q,cne_i);
                }
            }
        }
    }
    
    //    init_tree_weight_pair();
    for(i=0;i<edges_num;i++){
        printf("%d:%f\n",i,tree_edges_weights[i]);
    }
    exit(-4);
    return tree_edges_weights;
}

double * betwenness_heuristic(struct graph * g){
    bool * is_articulation_point=malloc(sizeof(bool)*g->nodes.size);
    struct list* connected_components=tarjan_rec_undir(g,is_articulation_point);
    struct list* tree_edges=connected_components_to_tree(g,connected_components,is_articulation_point);
    double *tree_weights=compute_component_tree_weights(g,tree_edges,connected_components);
    double * ret_val=(double *)malloc(sizeof(double)*g->nodes.size);
    int i=0;
    struct node_list * graph_iterator;
    for(graph_iterator=g->nodes.head;graph_iterator!=0;graph_iterator=graph_iterator->next){
        struct node_graph * n= ( struct node_graph *)graph_iterator->content;
        if(is_articulation_point[i]){
            struct node_list * tree_edge_iterator;
            double counter=0;
            double weight_sum=0;
            for(tree_edge_iterator=tree_edges->head;tree_edge_iterator!=0;tree_edge_iterator=tree_edge_iterator->next){
                struct cc_node_edge * cne=(struct cc_node_edge *)tree_edge_iterator->content;
                if(cne->to==n){
                    counter++;
                    weight_sum+=tree_weights[cne->index];
                } 
            } 
            ret_val[i]=counter-1;
        }else {
            ret_val[i]=0;
        }
        i++;
    }
    
    
    /*
     struct node_list * ccs_iterator;
     for(ccs_iterator=connected_components->head;ccs_iterator!=0;ccs_iterator=ccs_iterator->next){
     struct connected_component * cc= ( struct connected_component *)ccs_iterator->content;
     double * tmp=betweeness_brandes(&(cc->g));
     int i;
     for(i=0;i<cc->map_size;i++){
     printf("Adding %f to %d\n",tmp[i],cc->mapping[i]);
     ret_val[cc->mapping[i]]+=tmp[i];
     }
     free(tmp);
     free(cc->mapping);
     free(cc);
     }*/
    return ret_val;
}




void main(){
    /* struct graph g;
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
     // betweeness_brandes(&g);
     struct list*  ti=tarjan_iter(&g);
     print_CCs(ti);
     reset_graph(&g);
     struct list*  tr=tarjan_rec(&g);
     print_CCs(tr);
     //0: 0.08333333333333333, 1: 0.25, 2: 0.25, 3: 0.125, 4: 0.041666666666666664
     */
    struct graph g1;
    init_graph(&g1);
    add_edge_graph(&g1,"A","B",1,0);
    add_edge_graph(&g1,"A","C",1,0);
    add_edge_graph(&g1,"B","D",1,0);
    add_edge_graph(&g1,"C","D",1,0);
    add_edge_graph(&g1,"E","D",1,0);
    add_edge_graph(&g1,"E","F",1,0);
    add_edge_graph(&g1,"F","G",1,0);
    add_edge_graph(&g1,"H","G",1,0);
    add_edge_graph(&g1,"I","G",1,0);
    add_edge_graph(&g1,"I","J",1,0);
    add_edge_graph(&g1,"I","K",1,0);
    add_edge_graph(&g1,"L","K",1,0); 
    add_edge_graph(&g1,"G","M",1,0); 
    add_edge_graph(&g1,"L","M",1,0); 
    add_edge_graph(&g1,"L","N",1,0);
    add_edge_graph(&g1,"N","M",1,0);
    
    double * bh=betweeness_brandes(&g1);
    double * bh_c=brandes_heuristic(&g1);
    int i;
    for(i=0;i<g1.nodes.size;i++){
        printf("%f %f\n",bh[i],  bh_c[i]);
    }
    free(bh);
    free(bh_c);
    free_graph(&g1);
    /*
     struct list*  ti1=tarjan_iter(&g1);
     print_CCs(ti1);
     reset_graph(&g1);
     struct list*  tr2=tarjan_rec(&g1);
     print_CCs(tr2);
     exit(0);
     struct graph g2;
     init_graph(&g2);
     add_edge_graph(&g2,"1","0",1);
     add_edge_graph(&g2,"0","2",1);
     add_edge_graph(&g2,"2","1",1);
     add_edge_graph(&g2,"0","3",1);
     add_edge_graph(&g2,"3","4",1);
     tr2=tarjan_rec(&g2);
     print_CCs(tr2);
     
     struct graph g3;
     init_graph(&g3);
     add_edge_graph(&g3,"0","1",1);
     add_edge_graph(&g3,"1","2",1);
     add_edge_graph(&g3,"2","3",1);
     tr2=tarjan_rec(&g3);
     print_CCs(tr2);
     
     
     struct graph g4;
     init_graph(&g4);
     add_edge_graph(&g4,"0","1",1);
     add_edge_graph(&g4,"1","2",1);
     add_edge_graph(&g4,"2","0",1);
     add_edge_graph(&g4,"1","3",1);
     add_edge_graph(&g4,"1","4",1);
     add_edge_graph(&g4,"1","6",1);
     add_edge_graph(&g4,"3","5",1);
     add_edge_graph(&g4,"4","5",1);
     tr2=tarjan_rec(&g4);
     print_CCs(tr2);
     
     struct graph g5;
     init_graph(&g5);
     add_edge_graph(&g5,"0","1",1);
     add_edge_graph(&g5,"0","3",1);
     add_edge_graph(&g5,"1","2",1);
     add_edge_graph(&g5,"1","4",1);
     add_edge_graph(&g5,"2","0",1);
     add_edge_graph(&g5,"2","6",1);
     add_edge_graph(&g5,"3","2",1);
     add_edge_graph(&g5,"4","5",1);
     add_edge_graph(&g5,"4","6",1);
     add_edge_graph(&g5,"5","6",1);
     add_edge_graph(&g5,"5","7",1);
     add_edge_graph(&g5,"5","8",1);
     add_edge_graph(&g5,"5","9",1);
     add_edge_graph(&g5,"6","4",1);
     add_edge_graph(&g5,"7","9",1);
     add_edge_graph(&g5,"8","9",1);
     add_edge_graph(&g5,"9","8",1);
     tr2=tarjan_rec(&g5);
     print_CCs(tr2);
     
     
     struct graph g6;
     init_graph(&g6);
     add_edge_graph(&g6,"0","1",1);
     add_edge_graph(&g6,"1","2",1);
     add_edge_graph(&g6,"2","3",1);
     add_edge_graph(&g6,"2","4",1);
     add_edge_graph(&g6,"3","0",1);
     add_edge_graph(&g6,"4","2",1);
     tr2=tarjan_rec(&g6);
     print_CCs(tr2);
     
     */ 
}
