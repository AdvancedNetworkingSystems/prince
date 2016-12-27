/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "algorithms.h"
#include <limits.h>
//pseudocode from https://en.wikipedia.org/wiki/Tarjan's_strongly_connected_components_algorithm

const int INFINITY=INT_MAX;
bool multithread=true;

void strongconnect(struct node_graph* v,int * current_index, struct list * s,int * bcc_id,struct list* connected_components, int * index,int * low_link,bool * on_stack);  
void DFS_iter(struct node_graph* u,int * current_index,struct list * s,int *bcc_id,
        struct list* connected_components,struct node_graph ** caller, struct node_list ** iterator,
        int * index,int * low_link,bool * on_stack);
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
    
    int current_index=0;
    int bcc_id=0;
    struct node_list * n =g->nodes.head;
    int node_num=g->nodes.size;
    int * index=( int *)malloc(sizeof(int)*node_num);
    int * low_link=( int *)malloc(sizeof(int)*node_num);
    bool * on_stack=( bool *)malloc(sizeof(bool)*node_num);
    int i;
    for(i=0;i<node_num;i++){
        index[i]=-1;
        low_link[i]=-1;
        on_stack[i]=false;
    }
    
    while(n!=0){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(index[ng->node_graph_id]<0){
            strongconnect(ng,&current_index,&s,&bcc_id,connected_components,index,low_link,on_stack);
        }
        n=n->next;
    }
    free(index);
    free(low_link);
    free(on_stack);
    return connected_components;
}

void strongconnect(struct node_graph* v,int * current_index, struct list * s,int * bcc_id,struct list* connected_components, int * index,int * low_link,bool * on_stack){
    index[v->node_graph_id]=*current_index;
    low_link[v->node_graph_id]=*current_index;
    (*current_index)++;
    enqueue_list(s,(void*)v);
    on_stack[v->node_graph_id]=true;
    struct node_list * nq=v->neighbours.head;
    for(nq=v->neighbours.head;nq!=0; nq=nq->next){
        struct node_graph * w=((struct edge_graph*)nq->content)->to;
        if(index[w->node_graph_id]<0){
            strongconnect(w,current_index,s,bcc_id,connected_components,index,low_link,on_stack);
            low_link[v->node_graph_id]=min(low_link[v->node_graph_id],low_link[w->node_graph_id]);
        }else if(on_stack[w->node_graph_id]){
            low_link[v->node_graph_id]=min(low_link[v->node_graph_id],index[w->node_graph_id]);
        }
        
    }
    if(low_link[v->node_graph_id]==index[v->node_graph_id]){
        struct node_graph *w=0;
        struct list * cc_list=( struct list * )malloc(sizeof( struct list ));
        init_list(cc_list);
        do{
            w= (struct node_graph *) pop_list(s);
            on_stack[w->node_graph_id]=false;
            enqueue_list(cc_list,w);
        }while(w!=v);
        if(cc_list->size>0){//TODO: fix for components
            struct connected_component * connected=( struct connected_component * )malloc(sizeof( struct connected_component ));
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
    int current_index=0;
    int bcc_id=0;
    struct list s;
    init_list(&s);
    struct node_graph ** caller=(struct node_graph **)malloc(sizeof(struct node_graph *)*g->nodes.size);
    struct node_list ** iterator=(struct node_list **)malloc(sizeof(struct node_list *)*g->nodes.size);
    int node_num=g->nodes.size;
    int * index=( int *)malloc(sizeof(int)*node_num);
    int * low_link=( int *)malloc(sizeof(int)*node_num);
    bool * on_stack=( bool *)malloc(sizeof(bool)*node_num);
    int i;
    for(i=0;i<node_num;i++){
        index[i]=-1;
        low_link[i]=-1;
        on_stack[i]=false;
    }
    while(n!=0){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(index[ng->node_graph_id]<0){
            DFS_iter(ng,&current_index,&s,&bcc_id,connected_components,caller,iterator,index,low_link,on_stack);
        }
        n=n->next;
    }
    free(caller);
    free(iterator);
    free(index);
    free(low_link);
    free(on_stack);
    return connected_components;
}

//https://www.researchgate.net/profile/Oscar_Karnalim/publication/303959022_Improving_Scalability_of_Java_Archive_Search_Engine_through_Recursion_Conversion_And_Multithreading/links/57c929ed08aefc4af350b37d.pdf?origin=publication_detail
//http://stackoverflow.com/questions/2292223/iterative-version-of-a-recursive-algorithm-is-slower
void DFS_iter(struct node_graph* u,int * current_index,struct list * s,int *bcc_id,
        struct list* connected_components,struct node_graph ** caller, struct node_list ** iterator,int * index,int * low_link,bool * on_stack){
    index[u->node_graph_id]=*current_index;
    low_link[u->node_graph_id]=*current_index;
    (*current_index)++;
    iterator[u->node_graph_id]=u->neighbours.head;
    caller[u->node_graph_id]=0;
    on_stack[u->node_graph_id]=true,
    enqueue_list(s,(void*)u);
    struct node_graph* last=u;
    while(true){
        if(iterator[last->node_graph_id]!=0){
            struct node_graph *w = ((struct edge_graph*)iterator[last->node_graph_id]->content)->to;
            iterator[last->node_graph_id]=iterator[last->node_graph_id]->next; 
            if(index[w->node_graph_id]<0){
                caller[w->node_graph_id]=last;
                index[w->node_graph_id]=*current_index;
                low_link[w->node_graph_id]=*current_index;
                (*current_index)++;
                enqueue_list(s,(void*)w);
                on_stack[w->node_graph_id]=true;
                iterator[w->node_graph_id]=w->neighbours.head;
                last=w;
            }else if(on_stack[w->node_graph_id]){
                low_link[last->node_graph_id]=min(low_link[last->node_graph_id],index[w->node_graph_id]);
            }
        }else{
            if(index[last->node_graph_id]==low_link[last->node_graph_id]){
                struct list * cc_list=( struct list * )malloc(sizeof( struct list ));
                init_list(cc_list);
                struct node_graph* top=0;
                do{
                    top =(struct node_graph*)pop_list(s);
                    if(top!=0){
                        enqueue_list(cc_list,top);
                        on_stack[top->node_graph_id]=false;                    
                    }
                }while(top!=0 && index[top->node_graph_id]!=index[last->node_graph_id]);
                if(cc_list->size>0){
                    struct connected_component * connected=( struct connected_component * )malloc(sizeof( struct connected_component ));
                    enqueue_list(connected_components,connected);
                    (*bcc_id)++;
                }else{
                    free(cc_list);
                }
                
            }
            struct node_graph *new_last=caller[last->node_graph_id];
            if(new_last!=0){
                low_link[new_last->node_graph_id]=min(low_link[new_last->node_graph_id],low_link[last->node_graph_id]);
                last=new_last;
            }else{
                break;
            }
        }
    }
}

//http://algo.uni-konstanz.de/publications/b-vspbc-08.pdf
//todo: check Algorithm 11 for weights?
double * betweeness_brandes(struct graph * g, bool endpoints,int ** traffic_matrix){
    struct priority_queue q;
    struct list S;
    init_priority_queue(&q);
    init_priority_queue(&q);
    init_list(&S);
    int node_num=g->nodes.size;
    double * dist=( double *)malloc(node_num*sizeof(double));
    struct list * pred=(struct list *)malloc(node_num*sizeof( struct list));
    int * sigma=( int *)malloc(node_num*sizeof(int));
    double * delta=( double *)malloc(node_num*sizeof(double));
    double * ret_val=( double *)malloc(node_num*sizeof(double));
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
            //struct list * tmp=pred +i;
            dist[i]=INFINITY;
            sigma[i]=0;
            delta[i]=0;
        }
        dist[s->node_graph_id]=0;
        sigma[s->node_graph_id]=1;
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
                    if(dist[w->node_graph_id]>(dist[v->node_graph_id]+weight)){
                        dist[w->node_graph_id]=dist[v->node_graph_id]+weight;
                        insert_or_update_priority_queue(&q,w,dist[w->node_graph_id]);
                        sigma[w->node_graph_id]=0;
                        clear_list(pred +w->node_graph_id);
                    }if(dist[w->node_graph_id]==(dist[v->node_graph_id]+weight)){
                        sigma[w->node_graph_id]= sigma[w->node_graph_id]+ sigma[v->node_graph_id];
                        enqueue_list( pred+w->node_graph_id,v);
                    }
                }
            }
        }
        if(traffic_matrix!=0){//endpoints included by default
            while(!is_empty_list(&S)){
                struct node_graph * w=(struct node_graph * )pop_list(&S);
                double communication_intensity=(double)traffic_matrix[w->node_graph_id][s->node_graph_id];
                
                delta[w->node_graph_id]+=communication_intensity;
                ret_val[s->node_graph_id]+=communication_intensity;
                struct node_list * node_iterator;
                for(node_iterator =pred[w->node_graph_id].head;node_iterator!=0;node_iterator=node_iterator->next){
                    struct node_graph * v=(struct node_graph*)node_iterator->content;
                    delta[v->node_graph_id]+=(delta[w->node_graph_id]/ ((double)sigma[w->node_graph_id]));
                }
                if(w!=s){
                    ret_val[w->node_graph_id]=ret_val[w->node_graph_id]+delta[w->node_graph_id];
                }
            }       
        }else if(endpoints){
            ret_val[s->node_graph_id]+=(node_num-1);
            while(!is_empty_list(&S)){
                struct node_graph * w=(struct node_graph * )pop_list(&S);
                struct node_list * node_iterator;
                for(node_iterator =pred[w->node_graph_id].head;node_iterator!=0;node_iterator=node_iterator->next){
                    struct node_graph * v=(struct node_graph*)node_iterator->content;
                    delta[v->node_graph_id]= delta[v->node_graph_id]+((((double)sigma[v->node_graph_id])/ ((double)sigma[w->node_graph_id]))*(1+delta[w->node_graph_id]));
                }
                if(w!=s){
                    ret_val[w->node_graph_id]=ret_val[w->node_graph_id]+delta[w->node_graph_id]+1;
                }
            }
        }else{
            while(!is_empty_list(&S)){
                struct node_graph * w=(struct node_graph * )pop_list(&S);
                struct node_list * node_iterator;
                for(node_iterator =pred[w->node_graph_id].head;node_iterator!=0;node_iterator=node_iterator->next){
                    struct node_graph * v=(struct node_graph*)node_iterator->content;
                    delta[v->node_graph_id]= delta[v->node_graph_id]+((((double)sigma[v->node_graph_id])/ ((double)sigma[w->node_graph_id]))*(1+delta[w->node_graph_id]));
                }
                if(w!=s){
                    ret_val[w->node_graph_id]=ret_val[w->node_graph_id]+delta[w->node_graph_id];
                }
            }
        }
    }
    free(dist);
    for( i =0;i<node_num;i++){
        clear_list(&pred[i]);
    }
    clear_list(pred);
    free(pred);
    free(sigma);
    free(delta);
    //https://github.com/networkx/networkx/blob/master/networkx/algorithms/centrality/betweenness.py#L329
    //rescaling for directed and k==n (k is sample number), normalized is default
    if(node_num>2&&traffic_matrix==0){
        double scale=1/(((double)(node_num-1))*((double)(node_num-2)));
        for( i =0;i<node_num;i++){
            ret_val[i]*=scale;
        }
    }
    return ret_val;
}
//From http://www.cs.umd.edu/class/fall2005/cmsc451/biconcomps.pdf
struct list*  tarjan_rec_undir(struct graph * g, bool * is_articulation_point){
    struct list * connected_components=( struct list * )malloc(sizeof( struct list ));
    init_list(connected_components);
    int node_num=g->nodes.size;
    int count=0;
    bool * visited=(bool*)malloc(node_num*sizeof(bool));
    bool * added=(bool*)malloc(node_num*sizeof(bool));
    struct node_graph ** parent=(struct node_graph **)malloc(sizeof(struct node_graph *)*g->nodes.size);
    struct list s;
    init_list(&s);
    int * d=(int*)malloc(node_num*sizeof(int));
    int * low=(int*)malloc(node_num*sizeof(int));
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
    visited[u->node_graph_id]=true;
    d[u->node_graph_id]=(*count);
    low[u->node_graph_id]=(*count);
    (*count)=(*count)+1;
    if(u->neighbours.size>0){
        struct node_list * edge_iterator;
        for(edge_iterator=u->neighbours.head;edge_iterator!=0;edge_iterator=edge_iterator->next){
            struct edge_graph * edge=(struct edge_graph*)edge_iterator->content;
            struct node_graph * v=edge->to;
            if(!visited[v->node_graph_id]){
                visited[v->node_graph_id]=true;
                struct edge_repr * er=init_edge_repr(u,v,edge->value);
                enqueue_list(s,er);
                parent[v->node_graph_id]=u;
                DFS_visit(v,s,d,low,visited,parent,count,added,is_articulation_point,node_num,connected_components);
                
                if(low[v->node_graph_id]>=d[u->node_graph_id]){
                    struct edge_repr * er_i=0;
                    struct list l;
                    init_list(&l);
                    do{
                        er_i=( struct edge_repr *)pop_list(s);
                        added[er_i->from->node_graph_id]=true;
                        added[er_i->to->node_graph_id]=true;
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
                        cc->weights=(int *)malloc(sizeof(int)*added_count);
                        for(i=0;i<added_count;i++){
                            cc->weights[i]=-1;
                        }
                        //cc->map_size=added_count;
                        cc->cutpoint=0;
                        while(!is_empty_list(&l)){
                            struct edge_repr * er_i=( struct edge_repr *)pop_list(&l);
                            int f=0,t=0;
                            add_edge_graph_return_node_indexes(&(cc->g), er_i->from->name, er_i->to->name, er_i->value,0,&f, &t);
                            cc->mapping[f]= er_i->from->node_graph_id;
                            cc->mapping[t]= er_i->to->node_graph_id;
                            free(er_i);
                        }
                        enqueue_list(connected_components,cc);
                    }
                    clear_list(&l);
                }
                if((u->neighbours.size>1)&&(parent[u->node_graph_id]!=0 && low[v->node_graph_id] >= d[u->node_graph_id])){
                    is_articulation_point[u->node_graph_id]=true;
                }
                low[u->node_graph_id]=min(low[u->node_graph_id],low[v->node_graph_id]);
                
            }else if((parent[u->node_graph_id]!=v)&&(d[v->node_graph_id]<d[u->node_graph_id])){
                struct edge_repr * er=init_edge_repr(u,v,edge->value);
                enqueue_list(s,er);
                low[u->node_graph_id]=min(low[u->node_graph_id],d[v->node_graph_id]);
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
    int * weight;
    bool normal;
};

struct cc_node_edge * init_cc_node_edge(struct connected_component * from,struct node_graph * to,int * weight,bool normal){
    struct cc_node_edge * cne=(struct cc_node_edge * )malloc(sizeof(struct cc_node_edge ));
    cne->from=from;
    cne->to=to;
    cne->weight=weight;
    cne->normal=normal;
    return cne;
}


struct list*  connected_components_to_tree(struct graph * g, struct list* connected_components, bool * is_articulation_point){
    struct list* tree_edges=(struct list*)malloc(sizeof(struct list));
    init_list(tree_edges);
    struct node_list * ccs_iterator;
    struct node_graph ** nodes=(struct node_graph **)malloc(sizeof( struct node_graph *)*g->nodes.size);
    int i;
    struct node_list * node_iterator=g->nodes.head;
    for(i=0;i<g->nodes.size;i++){
        //visited[i]=false;
        nodes[i]=(struct node_graph *)node_iterator->content;
        node_iterator=node_iterator->next;
    }
    for(ccs_iterator=connected_components->head;ccs_iterator!=0;ccs_iterator=ccs_iterator->next){
        struct connected_component * cc= ( struct connected_component *)ccs_iterator->content;
        int art_points=0;
        struct node_graph * ng_cutpoint=0;
        int index=-1;
        
        struct node_list *n;
        i=0;
        for(n=cc->g.nodes.head;n!=0;n=n->next){
            struct node_graph *ng=(struct node_graph *)n->content;
            if(is_articulation_point[cc->mapping[i]]){
                int new_index=cc->mapping[i];
                ng_cutpoint=nodes[new_index];
                art_points++;
                index=i;
                struct cc_node_edge * cne=init_cc_node_edge(cc,nodes[new_index],cc->weights+i,true);
                enqueue_list(tree_edges,cne);
            }
            i++;
        }
        if(art_points==1){
            cc->cutpoint=ng_cutpoint;
            cc->cutpoint_index=index;
        }else{
            cc->cutpoint=0;
            cc->cutpoint_index=-1;
        }
    }
    free(nodes);
    return tree_edges;
}


//From http://algo.uni-konstanz.de/publications/pzedb-hsbcc-12.pdf
void compute_component_tree_weights(struct graph * g, struct list* tree_edges){
    struct list q;
    init_list(&q);
    int v_num=g->nodes.size;
    struct node_list * edge_iterator;
    for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
        struct cc_node_edge * cne=(struct cc_node_edge *)edge_iterator->content;
        if(cne->from->cutpoint!=0){
            enqueue_list(&q,cne);
        }
    }  
    while(!is_empty_list(&q)){
        struct cc_node_edge * cne=(struct cc_node_edge *)dequeue_list(&q);
        if(cne->normal){
            int size=cne->from->g.nodes.size-1;
            struct node_list * edge_iterator;
            for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                struct cc_node_edge * cne_i=(struct cc_node_edge*)edge_iterator->content;
                if(cne_i->from==cne->from&&(*cne_i->weight)!=-1){
                    size+=(v_num-(*cne_i->weight) - 1);
                }
            }
            (*cne->weight)=size;
            int count=0;
            struct cc_node_edge * t;
            for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                struct cc_node_edge * cne_i=(struct cc_node_edge*)edge_iterator->content;
                if(strcmp(cne_i->to->name,cne->to->name)==0&&(*cne_i->weight)==-1){
                    count++;
                    t=cne_i;
                    
                }
            }
            if(count==1){
                t->normal=false;
                enqueue_list(&q,t);
            }
        }else{            
            int size=0;
            struct node_list * edge_iterator;
            for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                struct cc_node_edge * cne_i=(struct cc_node_edge*)edge_iterator->content;
                if(cne_i->from!=cne->from&&strcmp(cne_i->to->name,cne->to->name)==0&&(*cne_i->weight)!=-1){
                    size+=(*cne_i->weight);
                }
            }
            (*cne->weight)=v_num-1-size;
            int count=0;
            struct cc_node_edge * t;
            for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                struct cc_node_edge * cne_i=(struct cc_node_edge*)edge_iterator->content;
                if(cne_i->from==cne->from&&(*cne_i->weight)==-1){
                    count++;
                    t=cne_i;
                }
            }
            if(count==1){
                t->normal=true;
                enqueue_list(&q,t);
            }
        }
    }
}

struct multithread_handle_cc_struct{
    struct connected_component * cc;
    int * node_num;
    bool *is_articulation_point;
    double * ret_val;
    pthread_t t;
};



double * handle_cc(  struct connected_component * cc,int node_num,bool *is_articulation_point){
    int cc_node_num=cc->g.nodes.size;
    int ** comm_matrix=(int **)malloc(sizeof(int*)*cc_node_num);
    int i;
    for( i=0;i<cc_node_num;i++){
        comm_matrix[i]=(int *)malloc(sizeof(int)*cc_node_num);
    }
    int j;
    for(i=0;i<cc_node_num;i++){
        for(j=0;j<cc_node_num;j++){
            if(i==j){
                comm_matrix[i][j]=0;
            }else{
                int new_i=cc->mapping[i],new_j=cc->mapping[j];
                bool is_i_ap=is_articulation_point[new_i];
                bool is_j_ap=is_articulation_point[new_j];
                if(!is_i_ap&&!is_j_ap){//0 art point
                    comm_matrix[i][j]=1;
                }else if(is_i_ap&&is_j_ap){
                    // reverse weight for both (rweight= |V| - weight -1) 
                    // it should be (rweight_i+1)*(rweight_j+1)
                    // the two ones are summed
                    comm_matrix[i][j]=((node_num-cc->weights[j])*(node_num-cc->weights[i]));
                }else{//one art point
                    // same as above
                    if(is_i_ap){
                        comm_matrix[i][j]=(node_num-(cc->weights[i]));
                    }else{
                        comm_matrix[i][j]=(node_num-(cc->weights[j]));
                    }
                }
            }
            
        } 
    } 
    double * ret_val=betweeness_brandes(&(cc->g),true,comm_matrix);        
    for(i=0;i<cc_node_num;i++){
        free(comm_matrix[i]);
    }
    free(comm_matrix);
    return ret_val;
}

void * run(void *arguments){
    struct multithread_handle_cc_struct *args = arguments;
    args->ret_val=handle_cc(args->cc,*args->node_num,args->is_articulation_point);
    return 0;
}


double * betwenness_heuristic(struct graph * g){
    bool * is_articulation_point=( bool * )malloc(sizeof(bool)*g->nodes.size);
    struct list* connected_components=tarjan_rec_undir(g,is_articulation_point);
    struct list* tree_edges=connected_components_to_tree(g,connected_components,is_articulation_point);
    compute_component_tree_weights(g,tree_edges);
    double * ret_val=(double *)malloc(sizeof(double)*g->nodes.size);
    int node_num=g->nodes.size;
    int i=0;
    for(i=0;i<g->nodes.size;i++)
        ret_val[i]=0;
    i=0;
    struct node_list * graph_iterator;
    for(graph_iterator=g->nodes.head;graph_iterator!=0;graph_iterator=graph_iterator->next){
        
        struct node_graph * n= ( struct node_graph *)graph_iterator->content;
        if(is_articulation_point[i]){
            struct node_list * tree_edge_iterator;
            double weight_sum=0;
            for(tree_edge_iterator=tree_edges->head;tree_edge_iterator!=0;tree_edge_iterator=tree_edge_iterator->next){
                struct cc_node_edge * cne=(struct cc_node_edge *)tree_edge_iterator->content;
                if(cne->to==n){
                    //counter++;
                    //give that DBV)+1+DBV(=|V|, BC^inter = (|V|-1+DBV( )*DBV(
                    // weight_sum+=tree_weights[cne->index]*(node_num-1-tree_weights[cne->index]);
                    weight_sum+=(*cne->weight)*(node_num-1-(*cne->weight));
                } 
            } 
            ret_val[i]-=weight_sum;
        }else {
            ret_val[i]=0;
        }
        i++;
    }
    
    struct node_list * ccs_iterator;
    if(multithread){
        int i=0;
        int cc_num=connected_components->size;
        struct multithread_handle_cc_struct * args=(struct multithread_handle_cc_struct *)malloc(sizeof(struct multithread_handle_cc_struct )*cc_num);
        for(ccs_iterator=connected_components->head;ccs_iterator!=0;ccs_iterator=ccs_iterator->next){
            struct connected_component * cc= ( struct connected_component *)ccs_iterator->content;
            args[i].cc=cc;
            args[i].is_articulation_point=is_articulation_point;
            args[i].node_num=&node_num;
            args[i].ret_val=0;
            i++;
        }
        for( i=0;i<cc_num;i++)
            pthread_create(&args[i].t, NULL, &run, (void *)(args+i));
        
        for( i=0;i<cc_num;i++){
            pthread_join(args[i].t, NULL);
            int j;
            for(j=0;j<args[i].cc->g.nodes.size;j++){
                ret_val[args[i].cc->mapping[j]] += args[i].ret_val[j];
            }
            free(args[i].ret_val);
        }
        free(args);
    }else{
        
        for(ccs_iterator=connected_components->head;ccs_iterator!=0;ccs_iterator=ccs_iterator->next){
            struct connected_component * cc= ( struct connected_component *)ccs_iterator->content;
            double * partial=handle_cc( cc, node_num,is_articulation_point);
            int i;
            for(i=0;i<cc->g.nodes.size;i++){
                ret_val[cc->mapping[i]] += partial[i];
            }
            free(partial);
        }
    }
    
    if(node_num>2){
        double scale=1/(((double)(node_num-1))*((double)(node_num-2)));
        for( i =0;i<node_num;i++){
            ret_val[i]*=scale;
        }
    }
    
    while(!is_empty_list(tree_edges)){
        struct cc_node_edge * cne=( struct cc_node_edge *)dequeue_list(tree_edges);
        free(cne);
    }
    free(tree_edges);
    while(!is_empty_list(connected_components)){
        struct connected_component * cc=(struct connected_component *)dequeue_list(connected_components);
        free_graph(&(cc->g));
        free(cc->mapping);
        free(cc->weights);
        free(cc);
    }
    free(connected_components);
    free(is_articulation_point);
    return ret_val;
}




int main(){
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
    struct node_list *  nl;
    double * bh=betweeness_brandes(&g1,true,0);
    
    
    
    
    
    double * bh_c=betwenness_heuristic(&g1);
    //int i;
    //for(i=0;i<g1.nodes.size;i++){
    //     printf("%f %f\n",bh[i],  bh_c[i]);
    //}
    
    for(nl=g1.nodes.head;nl!=0;nl=nl->next){
        struct node_graph * ng=(struct node_graph*)nl->content;
        // printf("%s %f\n",ng->name,bh[j]);
        printf("%s:\t%f \t%f\t%d\n",ng->name,bh[ng->node_graph_id],  bh_c[ng->node_graph_id],bh[ng->node_graph_id]==bh_c[ng->node_graph_id]);
        
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
    return 0;
}
