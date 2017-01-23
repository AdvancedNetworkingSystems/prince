/* 
 * File:   brandes.c
 * Author: principale
 *
 * Created on January 18, 2017, 12:17 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "brandes.h"
#include "biconnected.h"

const int INFINITY=INT_MAX;
bool multithread=true;

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

struct cc_node_edge * clone_cc_node_edge(struct cc_node_edge * cne){
    struct cc_node_edge * cne_n=(struct cc_node_edge * )malloc(sizeof(struct cc_node_edge ));
    cne_n->from=cne->from;
    cne_n->to=cne->to;
    cne_n->weight=cne->weight;
    cne_n->normal=cne->normal;
    return cne_n;
}


struct list*  connected_components_to_tree(struct graph * g, struct list* connected_components, bool * is_articulation_point){
    struct list* tree_edges=(struct list*)malloc(sizeof(struct list));
    init_list(tree_edges);
    struct node_list * ccs_iterator;
    struct node_graph ** nodes=(struct node_graph **)malloc(sizeof( struct node_graph *)*g->nodes.size);
    int i;
    struct node_list * node_iterator=g->nodes.head;
    for(i=0;i<g->nodes.size;i++){
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
void compute_component_tree_weights(struct graph * g, struct list* tree_edges,int v_num){
    struct list q;
    init_list(&q);
    struct node_list * edge_iterator;
    for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
        struct cc_node_edge * cne=(struct cc_node_edge *)edge_iterator->content;
        if(cne->from->cutpoint!=0){
            cne=clone_cc_node_edge(cne);
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
                if(cne_i->from==cne->from&&(*cne_i->weight)!=-1&&cne_i->to!=cne->to){
                    size+=(v_num-(*cne_i->weight) - 1); //right one
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
                t=clone_cc_node_edge(t);
                t->normal=false;
                enqueue_list(&q,t);
            }
        }else{            
            int size= 0;
            struct node_list * edge_iterator;
            for(edge_iterator=tree_edges->head;edge_iterator!=0;edge_iterator=edge_iterator->next){
                struct cc_node_edge * cne_i=(struct cc_node_edge*)edge_iterator->content;
                if(strcmp(cne_i->to->name,cne->to->name)==0&&(*cne_i->weight)!=-1&&cne_i->from!=cne->from){
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
                t=clone_cc_node_edge(t);
                t->normal=true;
                enqueue_list(&q,t);
            }
        }
        free(cne);
    }
}




double * handle_cc(  struct connected_component * cc,int node_num,bool *is_articulation_point){
    int cc_node_num=cc->g.nodes.size;
    int ** comm_matrix=(int **)malloc(sizeof(int*)*cc_node_num);
    int i;
    for( i=0;i<cc_node_num;i++){
        comm_matrix[i]=(int *)malloc(sizeof(int)*cc_node_num);
    }
    for(i=0;i<cc_node_num;i++){
        int j;
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

struct multithread_handle_cc_struct{
    struct connected_component * cc;
    int * node_num;
    bool *is_articulation_point;
    double * ret_val;
    pthread_t t;
};


void * run_brandes_heu(void *arguments){
    struct multithread_handle_cc_struct *args = ( struct multithread_handle_cc_struct *)arguments;
    args->ret_val=handle_cc(args->cc,*args->node_num,args->is_articulation_point);
    return 0;
}


void compute_heuristic_wo_scale(struct graph * g, 
        struct list * connected_components,
        bool * is_articulation_point,double * ret_val, 
        int * connected_component_index,int cc_node_num, int cc_index){
    int node_num=g->nodes.size;
    int i;
    node_num=cc_node_num;
    
    struct list* tree_edges=connected_components_to_tree(g,connected_components,is_articulation_point);
    compute_component_tree_weights(g,tree_edges,node_num);
    i=0;
    struct node_list * graph_iterator;
    for(graph_iterator=g->nodes.head;graph_iterator!=0;graph_iterator=graph_iterator->next){
        struct node_graph * n= ( struct node_graph *)graph_iterator->content;
        if(connected_component_index[i]==cc_index){
            if(is_articulation_point[i]){
                struct node_list * tree_edge_iterator;
                double weight_sum=0;//-1;
                for(tree_edge_iterator=tree_edges->head;tree_edge_iterator!=0;tree_edge_iterator=tree_edge_iterator->next){
                    struct cc_node_edge * cne=(struct cc_node_edge *)tree_edge_iterator->content;
                    if(cne->to==n){
                        //give that DBV)+1+DBV(=|V|, BC^inter = (|V|-1+DBV( )*DBV(
                        // weight_sum+=tree_weights[cne->index]*(node_num-1-tree_weights[cne->index]);
                        // weight_sum+=(*cne->weight)*(node_num-1-(*cne->weight));
                        weight_sum+=(*cne->weight)*(node_num-1-(*cne->weight));
                    } 
                } 
                ret_val[i]-=weight_sum;
            }else {
                ret_val[i]=0;
            }
        }
        i++;
    }
    
    struct node_list * ccs_iterator;
    int cc_num=connected_components->size;
    if(multithread && cc_num>1){
        int i=0;
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
            pthread_create(&args[i].t, NULL, &run_brandes_heu, (void *)(args+i));
        
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
}

struct multithread_subgraph_struct{
    struct graph * g;
    struct list * ccs;
    bool * art_point;
    double * ret_val;
    int *  indexes;
    int * size;
    int cc_index;
    pthread_t t;
};

void * run_subgraph(void *arguments){
    struct multithread_subgraph_struct *args = ( struct multithread_subgraph_struct *)arguments;
    compute_heuristic_wo_scale(args->g,args->ccs,args->art_point,args->ret_val,args->indexes,(*args->size),args->cc_index);
    return 0;
}

double * betwenness_heuristic(struct graph * g, bool recursive){
    int node_num=g->nodes.size;
    bool * is_articulation_point=( bool * )malloc(sizeof(bool)*node_num);
    int * connected_component_indexes=( int * )malloc(sizeof(int)*node_num);
    double * ret_val=(double *)malloc(sizeof(double)*node_num);
    int i;
    for(i=0;i<node_num;i++){
        ret_val[i]=0;
        connected_component_indexes[i]=-1;
    }
    struct list* connected_components_subgraphs;
    if(recursive){
        connected_components_subgraphs=tarjan_rec_undir(g,is_articulation_point,connected_component_indexes);
    }else {
        connected_components_subgraphs=tarjan_iter_undir(g,is_articulation_point,connected_component_indexes);
    }
    int connected_component_index=0;
    int cc_num=connected_components_subgraphs->size;
    if(multithread && cc_num>1){
        i=0;
        struct multithread_subgraph_struct * args=(struct multithread_subgraph_struct *)malloc(sizeof(struct multithread_subgraph_struct )*cc_num);
        struct node_list * subgraph_iterator=connected_components_subgraphs->head;
        for(;subgraph_iterator!=0;subgraph_iterator=subgraph_iterator->next){
            struct sub_graph * sg=(struct sub_graph *)subgraph_iterator->content;
            args[i].g=g;
            args[i].ccs=&sg->connected_components;
            args[i].art_point=is_articulation_point;
            args[i].ret_val=ret_val;
            args[i].indexes=connected_component_indexes;
            args[i].size=&sg->size;
            args[i].cc_index=i;
            i++;
        }
        for( i=0;i<cc_num;i++)
            pthread_create(&args[i].t, NULL, &run_subgraph, (void *)(args+i));
        
        for( i=0;i<cc_num;i++){
            pthread_join(args[i].t, NULL);
        }
        free(args);
    }else{
        //struct graph * g, struct list * connected_components,bool * is_articulation_point,double * ret_val
        while(!is_empty_list(connected_components_subgraphs)){
            struct sub_graph * sg=(struct sub_graph *)dequeue_list(connected_components_subgraphs);
            compute_heuristic_wo_scale(g,&(sg->connected_components),
                    is_articulation_point,ret_val,connected_component_indexes,
                    sg->size,connected_component_index++);
        }
    }
    clear_list(connected_components_subgraphs);
    free(connected_components_subgraphs);
    free(is_articulation_point);
    free(connected_component_indexes);
    if(node_num>2){
        double scale=1/(((double)(node_num-1))*((double)(node_num-2)));
        for( i =0;i<node_num;i++){
            ret_val[i]*=scale;
        }
    }
    return ret_val;
}

//http://algo.uni-konstanz.de/publications/b-vspbc-08.pdf
double * betweeness_brandes(struct graph * g, bool endpoints,int ** traffic_matrix){
    struct priority_queue q;
    struct list S;
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
        //weighted shortest path (dijkstra)
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
                        sigma[w->node_graph_id]+= sigma[v->node_graph_id];
                        enqueue_list( pred+w->node_graph_id,v);
                    }
                }
            }
        }
        //accumulation
        if(traffic_matrix!=0){
            //endpoints included by default
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
            ret_val[s->node_graph_id]+=(S.size-1);
            while(!is_empty_list(&S)){
                struct node_graph * w=(struct node_graph * )pop_list(&S);
                struct node_list * node_iterator;
                double coeff=(1+delta[w->node_graph_id])/((double)sigma[w->node_graph_id]);
                for(node_iterator =pred[w->node_graph_id].head;node_iterator!=0;node_iterator=node_iterator->next){
                    struct node_graph * v=(struct node_graph*)node_iterator->content;
                    delta[v->node_graph_id]+=((double)sigma[v->node_graph_id])*coeff;
                }
                if(w!=s){
                    ret_val[w->node_graph_id]+=delta[w->node_graph_id]+1;
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
    struct node_list * nl=g->nodes.head;
    if(node_num>2&&traffic_matrix==0){
        double scale=1/(((double)(node_num-1))*((double)(node_num-2)));
        for( i =0;i<node_num;i++){
            struct node_graph* ng=(struct node_graph*)nl->content;
            ret_val[ng->node_graph_id]*=scale;
            nl=nl->next;
        }
        
    }
    return ret_val;
}

int main(){
    /*
     struct graph g1;
     init_graph(&g1);
     add_edge_graph(&g1,"0","1",7.3003332296,0);
     struct node_list *  nl;
     double * bh=betweeness_brandes(&g1,true,0);
     
     double * bh_c=betwenness_heuristic(&g1,false);
     double * bh_c2=betwenness_heuristic(&g1,true);
     for(nl=g1.nodes.head;nl!=0;nl=nl->next){
     struct node_graph * ng=(struct node_graph*)nl->content;
     
     printf("%s:\t%f \t%f\t%f\t%d\t%f\n",
     ng->name,bh[ng->node_graph_id], 
     bh_c[ng->node_graph_id],bh_c2[ng->node_graph_id],
     bh[ng->node_graph_id]==bh_c[ng->node_graph_id] && bh[ng->node_graph_id]==bh_c2[ng->node_graph_id],
     -bh[ng->node_graph_id]+bh_c[ng->node_graph_id]);
     
     }
     free(bh);
     free(bh_c);
     free(bh_c2);
     free_graph(&g1);
     return 0;*/
}
