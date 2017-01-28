/* 
 * File:   brandes.c
 * Author: principale
 *
 * Created on January 18, 2017, 12:17 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "brandes.h"
#include "biconnected.h"
#include "network_change.h"


//parameter initialization
bool multithread=true;
bool stop_computing_if_unchanged=true;

/**
 * Whether we are using heuristic in case of single connected component.
 * From test based on 100 cliques of 200 nodes, results are
 * heuristic: mean 0.21064373254776, var: 4.5616834192685475e-06
 * original: mean 0.21824131727218629, var 9.6816689202285048e-06
 * so @use_heu_on_single_biconnected is set to true,to improve performance
 */
bool use_heu_on_single_biconnected=true;

/*
 * used for rounding. approximation to E^(-9)
 * This is not perfectly working, i.e. compared to another language (like python
 * Networkx) some errors will be always present, and will be larger the largest
 * a graph is. So this is an imperfect solution due to different language
 * mathematical approximation. 
 */
float decimal_places=1000000000;

inline double round_decimal(double d){
    return roundf(d*decimal_places)/decimal_places;
}


const int INFINITY_DIST=INT_MAX;
/**
 * This function implements brandes algorithm. Given a weighted graph, 
 * it returns an array of value, in which for every node identifier there is
 * its betwenness centrality value. For each node, it computes all routes to all
 * other node, and since it is weighted, uses Dijkstra algorithm. 
 * After this, it computes for each link the contribute to the current node.
 * As last operation, a normalization is compute in the number of nodes ( see 
 * http://algo.uni-konstanz.de/publications/b-vspbc-08.pdf)
 * 
 * @param g A (undirected) graph for which we want to compute centrality 
 * of nodes.
 * @param endpoints Whether we want to include endpoints in final value. It is 
 * always used as true when calling this function with next parameter null.
 * @param traffic_matrix It is either 0 or a matrix of integers. In first case,
 * normal brandes algorithm run, in second the heuristic one, considering
 *  intra-component and  inter-component traffic
 * @return An array with betwenness centrality for each node
 */
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
            dist[i]=INFINITY_DIST;
            sigma[i]=0;
            delta[i]=0;
        }
        dist[s->node_graph_id]=0;
        sigma[s->node_graph_id]=1;
        insert_priority_queue(&q,(void*)s,0);
        /**
         * weighted shortest path (dijkstra)
         */
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
                ret_val[s->node_graph_id]+=communication_intensity;
                struct node_list * node_iterator;
                for(node_iterator =pred[w->node_graph_id].head;node_iterator!=0;node_iterator=node_iterator->next){
                    struct node_graph * v=(struct node_graph*)node_iterator->content;
                    delta[v->node_graph_id]+=((delta[w->node_graph_id]+communication_intensity)*(((double)sigma[v->node_graph_id])/ ((double)sigma[w->node_graph_id])));
                }
                if(w!=s){
                    ret_val[w->node_graph_id]+=delta[w->node_graph_id]+communication_intensity;
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
    //clear_list(pred);
    free(pred);
    free(sigma);
    free(delta);
    struct node_list * nl=g->nodes.head;
    if(node_num>2&&traffic_matrix==0){
        double scale=1/(((double)(node_num-1))*((double)(node_num-2)));
        for( i =0;i<node_num;i++){
            struct node_graph* ng=(struct node_graph*)nl->content;
            ret_val[ng->node_graph_id]*=scale;
            ret_val[ng->node_graph_id]=round_decimal(ret_val[ng->node_graph_id]);
            nl=nl->next;
        }
        
    }
    return ret_val;
}


// normal defines whether the pair is (B,v) (if false, (v,B)). Since ordering 
// would be a major computational effort, a boolean ("normal") indicates that.
struct cc_node_edge{
    struct connected_component * from;
    struct node_graph * to;
    int * weight;
    bool normal;
};

/**
 * Algorithm for heuristic  (not described explicitly in the paper)
 * It initializes a link from a cutpoint to a component.
 * 
 * @param from The connected component on the end of the link
 * @param to The node (in the connected component) originating the link
 * @param weight The edge weight
 * @param normal Whether in the computation represent a normal couple 
 * node-component or a component-node one. Used in compute_component_tree_weights
 * @return the newly created link
 */
struct cc_node_edge * init_cc_node_edge(struct connected_component * from,struct node_graph * to,int * weight,bool normal){
    struct cc_node_edge * cne=(struct cc_node_edge * )malloc(sizeof(struct cc_node_edge ));
    cne->from=from;
    cne->to=to;
    cne->weight=weight;
    cne->normal=normal;
    return cne;
}
/**
 * Algorithm for heuristic  (not described explicitly in the paper)
 * It initializes a link from a cutpoint to a component from a given one.
 * Since they are used as normal and reversed, we need to clone them to avoid
 * changing already collected link.
 * 
 * @param cne A base link to clone
 * @return a clone of the original link
 */
struct cc_node_edge * clone_cc_node_edge(struct cc_node_edge * cne){
    struct cc_node_edge * cne_n=(struct cc_node_edge * )malloc(sizeof(struct cc_node_edge ));
    cne_n->from=cne->from;
    cne_n->to=cne->to;
    cne_n->weight=cne->weight;
    cne_n->normal=cne->normal;
    return cne_n;
}

/**
 * Algorithm for heuristic  (not described explicitly in the paper)
 * Given a connected components list in graph and the list of articulation
 * point it creates a tree representing the inter-component traffic. It will be 
 * used to remove redundant traffic from cutpoints in final centrality value.
 * 
 * @param g A weighted graph. It needs to be connected or either you have to 
 * call this function for every connected component (i.e. for every subgraph)
 * @param connected_components The list of the connected components inside 
 * the graph (or either a subgraph of it)
 * @param is_articulation_point Array of boolean which tells if node with given
 * id is an articulation point
 * @return A tree representing the flow of routes
 */
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
/**
 * Algorithm for heuristic 
 * Computes the inter components (biconnected) traffic and removes the redundant
 * one from the final values. For each cutpoint every component value is 
 * assessed in order to retrieve the traffic that is computed more than once on
 * it.
 * The implementation is slightly different from original, for two values, which
 * are 
 *      size+=(v_num-(*cne_i->weight) - 1);
 * and
 *      int size= 0;
 * in which first value should be 0 and the latter should be 1.
 * 
 * 
 * @param g A weighted graph. It needs to be connected or either you have to 
 * call this function for every connected component (i.e. for every subgraph)
 * @param tree_edges The tree of the biconnected components, result of 
 * connected_components_to_tree
 * @param v_num number of vertex in the given graph (or connected subgraph)
 */
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



/**
 * Algorithm for heuristic (not described explicitly in the paper)
 * Given a biconnected component in the graph it computes the traffic matrix and 
 * finally returns the gross centrality (the redundancy of traffic is present).
 * This function runs concurrently on all biconnected components, if they are
 * more than 1.
 * 
 * @param cc The connected component
 * @param node_num The total number of node in the whole graph (not only 
 * the connected subgraph amount)
 * @param is_articulation_point Array of boolean which tells if node with given
 * id is an articulation point
 * @return The gross centrality of the biconnected component.
 */
double * compute_traffic_matrix_and_centrality(  struct connected_component * cc,int node_num,bool *is_articulation_point){
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

struct multithread_compute_traffic_matrix_and_centrality_struct{
    struct connected_component * cc;
    int * node_num;
    bool *is_articulation_point;
    double * ret_val;
    pthread_t t;
};

/**
 * Algorithm for heuristic (not described explicitly in the paper)
 * Helper function to compute brandes centrality concurrently.
 * 
 * @param arguments struct wrapper for arguments of 
 * compute_traffic_matrix_and_centrality, i.e. connected component, node number,
 * array for betwenness value, reference thread
 * @return nothing, it respects the typing for a pthread thread
 */
void * run_brandes_heu(void *arguments){
    struct multithread_compute_traffic_matrix_and_centrality_struct *args = ( struct multithread_compute_traffic_matrix_and_centrality_struct *)arguments;
    args->ret_val=compute_traffic_matrix_and_centrality(args->cc,*args->node_num,args->is_articulation_point);
    return 0;
}

/**
 *  Algorithm for heuristic (not described explicitly in the paper)
 *  It computes the centrality for every connected subgraph.
 *  Given the connected subgraph, it performs the tree decomposition of 
 *  biconnected components. Then computes the BC_inter, i.e. the inter 
 *  biconnected components traffic. Then concurrently, if possible, computes 
 *  the traffic matrix for each component and the precise brandes value.
 *  It may run concurrently.
 *  
 * @param g A weighted graph
 * @param connected_components List of connected components for a specific 
 * connected subgraph
 * @param is_articulation_point Array of boolean which tells if node with given
 * id is an articulation point
 * @param bc The final centrality, an array whose indexes are node id
 * @param connected_component_index an array that specifies to which connected
 * component a node belongs
 * @param cc_node_num the number of nodes in the connected subgraph
 * @param cc_index an identifier (index) of the connected subgraph
 */
void compute_heuristic_wo_scale(struct graph * g, 
        struct list * connected_components,
        bool * is_articulation_point,double * bc, 
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
                bc[i]-=weight_sum;
            }else {
                bc[i]=0;
            }
        }
        i++;
    }
    
    struct node_list * ccs_iterator;
    int bcc_num=connected_components->size;
    if(multithread && bcc_num>1){
        int i=0;
        struct multithread_compute_traffic_matrix_and_centrality_struct * args=(struct multithread_compute_traffic_matrix_and_centrality_struct *)malloc(sizeof(struct multithread_compute_traffic_matrix_and_centrality_struct )*bcc_num);
        for(ccs_iterator=connected_components->head;ccs_iterator!=0;ccs_iterator=ccs_iterator->next){
            struct connected_component * cc= ( struct connected_component *)ccs_iterator->content;
            args[i].cc=cc;
            args[i].is_articulation_point=is_articulation_point;
            args[i].node_num=&node_num;
            args[i].ret_val=0;
            i++;
        }
        for( i=0;i<bcc_num;i++)
            pthread_create(&args[i].t, NULL, &run_brandes_heu, (void *)(args+i));
        
        for( i=0;i<bcc_num;i++){
            pthread_join(args[i].t, NULL);
            int j;
            for(j=0;j<args[i].cc->g.nodes.size;j++){
                bc[args[i].cc->mapping[j]] += args[i].ret_val[j];
            }
            free(args[i].ret_val);
        }
        free(args);
    }else{
        for(ccs_iterator=connected_components->head;ccs_iterator!=0;ccs_iterator=ccs_iterator->next){
            struct connected_component * cc= ( struct connected_component *)ccs_iterator->content;
            double * partial=compute_traffic_matrix_and_centrality( cc, node_num,is_articulation_point);
            int i;
            for(i=0;i<cc->g.nodes.size;i++){
                bc[cc->mapping[i]] += partial[i];
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
    double * bc;
    int *  indexes;
    int * size;
    int cc_index;
    pthread_t t;
};

/**
 *  Algorithm for heuristic (not described explicitly in the paper)
 *  Helper function that runs brandes heuristic on connected subgraph
 * @param arguments the list of arguments, the original graph, the list of 
 * biconnected components of current connected subgraph, the articulation point, 
 * the betwenness results array, the list of id that tells to which connected 
 * component a node belong, the size of the connected graph, the size of the 
 * current subgraph, the index of it and the current thread.
 * @return nothing, it respects the typing for a pthread thread
 */
void * run_subgraph(void *arguments){
    struct multithread_subgraph_struct *args = ( struct multithread_subgraph_struct *)arguments;
    compute_heuristic_wo_scale(args->g,args->ccs,args->art_point,args->bc,args->indexes,(*args->size),args->cc_index);
    return 0;
}
/**
 * Algorithm for heuristic 
 * Wrapper for all function above. It returns the correct centrality, it is
 * semantically the same as betweeness_brandes. 
 * 
 * @param g A weighted graph
 * @param recursive whether we want to use the recursive or iterative approach
 * @return  An array with betwenness centrality for each node 
 */
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
    
    int biconnected_component_num=-1,result_size=-1;
    float standard_deviation=-1;
    
    if(stop_computing_if_unchanged){
        //if we rely on old values when network 
        //is not changed
        char ** old_names=0;
        double *  old_ret_val=is_network_changed(connected_components_subgraphs,
                &biconnected_component_num,&standard_deviation,&result_size,&old_names);
        if(old_ret_val!=0){
            //free everything is behind
            while(!is_empty_list(connected_components_subgraphs)){
                struct list* tmp=( struct list*)dequeue_list(connected_components_subgraphs);
                while(!is_empty_list(tmp)){
                    struct connected_component * cc=( struct connected_component *)dequeue_list(tmp);
                    free_graph(&cc->g);
                    free(cc->mapping);
                    free(cc->weights);
                    free(cc);
                }
                
                free(tmp);
            }
            clear_list(connected_components_subgraphs);
            free(connected_components_subgraphs);
            free(is_articulation_point);
            free(connected_component_indexes);
            if(node_num==result_size){
                copy_old_values(old_ret_val,ret_val,old_names,result_size,&g->nodes);
            }
            free(old_ret_val);
            if(old_names!=0){
                for (i=0;i<result_size;i++)
                    free(old_names[i]);
                free(old_names);
            }
            return ret_val;
        }
        if(old_names!=0){
            for (i=0;i<result_size;i++)
                free(old_names[i]);
            free(old_names);
        }
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
            args[i].bc=ret_val;
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
        
        struct sub_graph * sg=(struct sub_graph *)dequeue_list(connected_components_subgraphs);
        if(sg->connected_components.size>1||use_heu_on_single_biconnected){;
        compute_heuristic_wo_scale(g,&(sg->connected_components),
                is_articulation_point,ret_val,connected_component_indexes,
                sg->size,connected_component_index++);
        }else {
            clear_list(connected_components_subgraphs);
            free(connected_components_subgraphs);
            free(is_articulation_point);
            free(connected_component_indexes);
            free(ret_val);
            return betweeness_brandes(g,true,0);
        }
        
        /*   //struct graph * g, struct list * connected_components,bool * is_articulation_point,double * ret_val
         while(!is_empty_list(connected_components_subgraphs)){
         struct sub_graph * sg=(struct sub_graph *)dequeue_list(connected_components_subgraphs);
         compute_heuristic_wo_scale(g,&(sg->connected_components),
         is_articulation_point,ret_val,connected_component_indexes,
         sg->size,connected_component_index++);
         }*/
        
    }
    
    if(node_num>2){
        double scale=1/(((double)(node_num-1))*((double)(node_num-2)));
        for( i =0;i<node_num;i++){
            ret_val[i]*=scale;
            ret_val[i]=round_decimal(ret_val[i]);
            
        }
    }
    //if we are storing values for next computation
    if(stop_computing_if_unchanged){
        write_file(biconnected_component_num,standard_deviation,node_num,ret_val,&g->nodes);
    }
    
    
    clear_list(connected_components_subgraphs);
    free(connected_components_subgraphs);
    free(is_articulation_point);
    free(connected_component_indexes);
    
    
    return ret_val;
}
/*
 int main(){
 //double d=sqrt(2);
 //int decimal_places=1000000000;
 //double d1=roundf(d*decimal_places)/decimal_places;
 //decimal_places=1000000;
 //double d2=roundf(d*decimal_places)/decimal_places;
 //printf("%1.50f\n",d1-d);
 struct graph g1;
 init_graph(&g1);
 add_edge_graph(&g1,"0","42",1,0);
 add_edge_graph(&g1,"1","545",1,0);
 add_edge_graph(&g1,"1","130",1,0);
 add_edge_graph(&g1,"1","839",1,0);
 add_edge_graph(&g1,"1","428",1,0);
 add_edge_graph(&g1,"1","371",1,0);
 add_edge_graph(&g1,"1","92",1,0);
 add_edge_graph(&g1,"2","793",1,0);
 add_edge_graph(&g1,"2","371",1,0);
 add_edge_graph(&g1,"2","534",1,0);
 add_edge_graph(&g1,"3","280",1,0);
 add_edge_graph(&g1,"3","371",1,0);
 add_edge_graph(&g1,"4","374",1,0);
 add_edge_graph(&g1,"5","371",1,0);
 add_edge_graph(&g1,"5","655",1,0);
 add_edge_graph(&g1,"6","545",1,0);
 add_edge_graph(&g1,"7","281",1,0);
 add_edge_graph(&g1,"8","371",1,0);
 add_edge_graph(&g1,"8","653",1,0);
 add_edge_graph(&g1,"9","882",1,0);
 add_edge_graph(&g1,"9","605",1,0);
 add_edge_graph(&g1,"9","766",1,0);
 add_edge_graph(&g1,"10","540",1,0);
 add_edge_graph(&g1,"10","306",1,0);
 add_edge_graph(&g1,"10","534",1,0);
 add_edge_graph(&g1,"10","247",1,0);
 add_edge_graph(&g1,"10","729",1,0);
 add_edge_graph(&g1,"10","668",1,0);
 add_edge_graph(&g1,"11","947",1,0);
 add_edge_graph(&g1,"11","534",1,0);
 add_edge_graph(&g1,"12","637",1,0);
 add_edge_graph(&g1,"13","641",1,0);
 add_edge_graph(&g1,"13","114",1,0);
 add_edge_graph(&g1,"14","130",1,0);
 add_edge_graph(&g1,"14","534",1,0);
 add_edge_graph(&g1,"14","282",1,0);
 add_edge_graph(&g1,"14","545",1,0);
 add_edge_graph(&g1,"14","420",1,0);
 add_edge_graph(&g1,"14","682",1,0);
 add_edge_graph(&g1,"14","306",1,0);
 add_edge_graph(&g1,"14","437",1,0);
 add_edge_graph(&g1,"14","950",1,0);
 add_edge_graph(&g1,"14","952",1,0);
 add_edge_graph(&g1,"14","191",1,0);
 add_edge_graph(&g1,"14","64",1,0);
 add_edge_graph(&g1,"14","194",1,0);
 add_edge_graph(&g1,"14","324",1,0);
 add_edge_graph(&g1,"14","582",1,0);
 add_edge_graph(&g1,"14","970",1,0);
 add_edge_graph(&g1,"14","461",1,0);
 add_edge_graph(&g1,"14","466",1,0);
 add_edge_graph(&g1,"14","471",1,0);
 add_edge_graph(&g1,"14","605",1,0);
 add_edge_graph(&g1,"14","371",1,0);
 add_edge_graph(&g1,"14","888",1,0);
 add_edge_graph(&g1,"14","121",1,0);
 add_edge_graph(&g1,"14","634",1,0);
 add_edge_graph(&g1,"14","382",1,0);
 add_edge_graph(&g1,"15","801",1,0);
 add_edge_graph(&g1,"15","972",1,0);
 add_edge_graph(&g1,"15","878",1,0);
 add_edge_graph(&g1,"15","371",1,0);
 add_edge_graph(&g1,"15","659",1,0);
 add_edge_graph(&g1,"15","534",1,0);
 add_edge_graph(&g1,"15","471",1,0);
 add_edge_graph(&g1,"15","468",1,0);
 add_edge_graph(&g1,"16","281",1,0);
 add_edge_graph(&g1,"16","466",1,0);
 add_edge_graph(&g1,"16","291",1,0);
 add_edge_graph(&g1,"16","29",1,0);
 add_edge_graph(&g1,"16","682",1,0);
 add_edge_graph(&g1,"17","997",1,0);
 add_edge_graph(&g1,"17","582",1,0);
 add_edge_graph(&g1,"17","42",1,0);
 add_edge_graph(&g1,"17","781",1,0);
 add_edge_graph(&g1,"17","686",1,0);
 add_edge_graph(&g1,"17","303",1,0);
 add_edge_graph(&g1,"17","371",1,0);
 add_edge_graph(&g1,"17","437",1,0);
 add_edge_graph(&g1,"17","280",1,0);
 add_edge_graph(&g1,"17","605",1,0);
 add_edge_graph(&g1,"17","405",1,0);
 add_edge_graph(&g1,"18","306",1,0);
 add_edge_graph(&g1,"18","723",1,0);
 add_edge_graph(&g1,"19","130",1,0);
 add_edge_graph(&g1,"19","775",1,0);
 add_edge_graph(&g1,"19","395",1,0);
 add_edge_graph(&g1,"19","911",1,0);
 add_edge_graph(&g1,"19","659",1,0);
 add_edge_graph(&g1,"19","405",1,0);
 add_edge_graph(&g1,"19","534",1,0);
 add_edge_graph(&g1,"19","280",1,0);
 add_edge_graph(&g1,"19","281",1,0);
 add_edge_graph(&g1,"19","282",1,0);
 add_edge_graph(&g1,"19","671",1,0);
 add_edge_graph(&g1,"19","178",1,0);
 add_edge_graph(&g1,"19","306",1,0);
 add_edge_graph(&g1,"19","437",1,0);
 add_edge_graph(&g1,"19","121",1,0);
 add_edge_graph(&g1,"19","825",1,0);
 add_edge_graph(&g1,"19","415",1,0);
 add_edge_graph(&g1,"19","445",1,0);
 add_edge_graph(&g1,"19","574",1,0);
 add_edge_graph(&g1,"19","704",1,0);
 add_edge_graph(&g1,"19","449",1,0);
 add_edge_graph(&g1,"19","66",1,0);
 add_edge_graph(&g1,"19","971",1,0);
 add_edge_graph(&g1,"19","120",1,0);
 add_edge_graph(&g1,"19","723",1,0);
 add_edge_graph(&g1,"19","85",1,0);
 add_edge_graph(&g1,"19","471",1,0);
 add_edge_graph(&g1,"19","472",1,0);
 add_edge_graph(&g1,"19","217",1,0);
 add_edge_graph(&g1,"19","605",1,0);
 add_edge_graph(&g1,"19","979",1,0);
 add_edge_graph(&g1,"19","997",1,0);
 add_edge_graph(&g1,"19","371",1,0);
 add_edge_graph(&g1,"19","248",1,0);
 add_edge_graph(&g1,"19","761",1,0);
 add_edge_graph(&g1,"20","972",1,0);
 add_edge_graph(&g1,"21","130",1,0);
 add_edge_graph(&g1,"22","130",1,0);
 add_edge_graph(&g1,"22","371",1,0);
 add_edge_graph(&g1,"23","471",1,0);
 add_edge_graph(&g1,"24","184",1,0);
 add_edge_graph(&g1,"24","42",1,0);
 add_edge_graph(&g1,"24","605",1,0);
 add_edge_graph(&g1,"24","574",1,0);
 add_edge_graph(&g1,"25","198",1,0);
 add_edge_graph(&g1,"26","84",1,0);
 add_edge_graph(&g1,"27","682",1,0);
 add_edge_graph(&g1,"27","371",1,0);
 add_edge_graph(&g1,"28","437",1,0);
 add_edge_graph(&g1,"29","976",1,0);
 add_edge_graph(&g1,"29","971",1,0);
 add_edge_graph(&g1,"29","723",1,0);
 add_edge_graph(&g1,"29","405",1,0);
 add_edge_graph(&g1,"29","534",1,0);
 add_edge_graph(&g1,"29","471",1,0);
 add_edge_graph(&g1,"29","729",1,0);
 add_edge_graph(&g1,"30","545",1,0);
 add_edge_graph(&g1,"30","855",1,0);
 add_edge_graph(&g1,"31","683",1,0);
 add_edge_graph(&g1,"31","36",1,0);
 add_edge_graph(&g1,"32","492",1,0);
 add_edge_graph(&g1,"33","226",1,0);
 add_edge_graph(&g1,"33","534",1,0);
 add_edge_graph(&g1,"33","582",1,0);
 add_edge_graph(&g1,"33","71",1,0);
 add_edge_graph(&g1,"33","130",1,0);
 add_edge_graph(&g1,"33","306",1,0);
 add_edge_graph(&g1,"33","563",1,0);
 add_edge_graph(&g1,"33","723",1,0);
 add_edge_graph(&g1,"34","320",1,0);
 add_edge_graph(&g1,"34","876",1,0);
 add_edge_graph(&g1,"34","574",1,0);
 add_edge_graph(&g1,"35","281",1,0);
 add_edge_graph(&g1,"36","129",1,0);
 add_edge_graph(&g1,"36","420",1,0);
 add_edge_graph(&g1,"36","389",1,0);
 add_edge_graph(&g1,"36","545",1,0);
 add_edge_graph(&g1,"36","587",1,0);
 add_edge_graph(&g1,"36","718",1,0);
 add_edge_graph(&g1,"36","303",1,0);
 add_edge_graph(&g1,"36","306",1,0);
 add_edge_graph(&g1,"36","563",1,0);
 add_edge_graph(&g1,"36","334",1,0);
 add_edge_graph(&g1,"36","245",1,0);
 add_edge_graph(&g1,"36","983",1,0);
 add_edge_graph(&g1,"36","371",1,0);
 add_edge_graph(&g1,"36","921",1,0);
 add_edge_graph(&g1,"36","351",1,0);
 add_edge_graph(&g1,"36","622",1,0);
 add_edge_graph(&g1,"37","786",1,0);
 add_edge_graph(&g1,"37","85",1,0);
 add_edge_graph(&g1,"38","457",1,0);
 add_edge_graph(&g1,"38","989",1,0);
 add_edge_graph(&g1,"39","306",1,0);
 add_edge_graph(&g1,"40","42",1,0);
 add_edge_graph(&g1,"40","571",1,0);
 add_edge_graph(&g1,"40","471",1,0);
 add_edge_graph(&g1,"41","466",1,0);
 add_edge_graph(&g1,"42","129",1,0);
 add_edge_graph(&g1,"42","130",1,0);
 add_edge_graph(&g1,"42","321",1,0);
 add_edge_graph(&g1,"42","521",1,0);
 add_edge_graph(&g1,"42","192",1,0);
 add_edge_graph(&g1,"42","704",1,0);
 add_edge_graph(&g1,"42","142",1,0);
 add_edge_graph(&g1,"42","787",1,0);
 add_edge_graph(&g1,"42","405",1,0);
 add_edge_graph(&g1,"42","534",1,0);
 add_edge_graph(&g1,"42","281",1,0);
 add_edge_graph(&g1,"42","282",1,0);
 add_edge_graph(&g1,"42","540",1,0);
 add_edge_graph(&g1,"42","159",1,0);
 add_edge_graph(&g1,"42","896",1,0);
 add_edge_graph(&g1,"42","420",1,0);
 add_edge_graph(&g1,"42","682",1,0);
 add_edge_graph(&g1,"42","303",1,0);
 add_edge_graph(&g1,"42","306",1,0);
 add_edge_graph(&g1,"42","563",1,0);
 add_edge_graph(&g1,"42","437",1,0);
 add_edge_graph(&g1,"42","826",1,0);
 add_edge_graph(&g1,"42","960",1,0);
 add_edge_graph(&g1,"42","449",1,0);
 add_edge_graph(&g1,"42","582",1,0);
 add_edge_graph(&g1,"42","968",1,0);
 add_edge_graph(&g1,"42","331",1,0);
 add_edge_graph(&g1,"42","332",1,0);
 add_edge_graph(&g1,"42","589",1,0);
 add_edge_graph(&g1,"42","334",1,0);
 add_edge_graph(&g1,"42","463",1,0);
 add_edge_graph(&g1,"42","466",1,0);
 add_edge_graph(&g1,"42","467",1,0);
 add_edge_graph(&g1,"42","462",1,0);
 add_edge_graph(&g1,"42","471",1,0);
 add_edge_graph(&g1,"42","220",1,0);
 add_edge_graph(&g1,"42","605",1,0);
 add_edge_graph(&g1,"42","351",1,0);
 add_edge_graph(&g1,"42","609",1,0);
 add_edge_graph(&g1,"42","997",1,0);
 add_edge_graph(&g1,"42","536",1,0);
 add_edge_graph(&g1,"42","764",1,0);
 add_edge_graph(&g1,"42","618",1,0);
 add_edge_graph(&g1,"42","752",1,0);
 add_edge_graph(&g1,"42","241",1,0);
 add_edge_graph(&g1,"42","114",1,0);
 add_edge_graph(&g1,"42","371",1,0);
 add_edge_graph(&g1,"42","501",1,0);
 add_edge_graph(&g1,"42","869",1,0);
 add_edge_graph(&g1,"42","504",1,0);
 add_edge_graph(&g1,"42","892",1,0);
 add_edge_graph(&g1,"42","490",1,0);
 add_edge_graph(&g1,"42","254",1,0);
 add_edge_graph(&g1,"42","597",1,0);
 add_edge_graph(&g1,"43","328",1,0);
 add_edge_graph(&g1,"43","864",1,0);
 add_edge_graph(&g1,"44","534",1,0);
 add_edge_graph(&g1,"44","471",1,0);
 add_edge_graph(&g1,"45","449",1,0);
 add_edge_graph(&g1,"45","130",1,0);
 add_edge_graph(&g1,"45","534",1,0);
 add_edge_graph(&g1,"46","582",1,0);
 add_edge_graph(&g1,"47","303",1,0);
 add_edge_graph(&g1,"47","767",1,0);
 add_edge_graph(&g1,"47","244",1,0);
 add_edge_graph(&g1,"47","351",1,0);
 add_edge_graph(&g1,"48","464",1,0);
 add_edge_graph(&g1,"48","471",1,0);
 add_edge_graph(&g1,"49","571",1,0);
 add_edge_graph(&g1,"49","471",1,0);
 add_edge_graph(&g1,"50","705",1,0);
 add_edge_graph(&g1,"50","130",1,0);
 add_edge_graph(&g1,"50","420",1,0);
 add_edge_graph(&g1,"50","114",1,0);
 add_edge_graph(&g1,"50","534",1,0);
 add_edge_graph(&g1,"50","471",1,0);
 add_edge_graph(&g1,"50","452",1,0);
 add_edge_graph(&g1,"51","960",1,0);
 add_edge_graph(&g1,"51","582",1,0);
 add_edge_graph(&g1,"52","121",1,0);
 add_edge_graph(&g1,"52","130",1,0);
 add_edge_graph(&g1,"52","723",1,0);
 add_edge_graph(&g1,"52","341",1,0);
 add_edge_graph(&g1,"52","334",1,0);
 add_edge_graph(&g1,"53","130",1,0);
 add_edge_graph(&g1,"53","733",1,0);
 add_edge_graph(&g1,"54","449",1,0);
 add_edge_graph(&g1,"54","371",1,0);
 add_edge_graph(&g1,"54","733",1,0);
 add_edge_graph(&g1,"54","534",1,0);
 add_edge_graph(&g1,"54","303",1,0);
 add_edge_graph(&g1,"55","545",1,0);
 add_edge_graph(&g1,"55","130",1,0);
 add_edge_graph(&g1,"55","235",1,0);
 add_edge_graph(&g1,"55","466",1,0);
 add_edge_graph(&g1,"55","371",1,0);
 add_edge_graph(&g1,"55","723",1,0);
 add_edge_graph(&g1,"55","471",1,0);
 add_edge_graph(&g1,"55","91",1,0);
 add_edge_graph(&g1,"56","371",1,0);
 add_edge_graph(&g1,"56","582",1,0);
 add_edge_graph(&g1,"57","306",1,0);
 add_edge_graph(&g1,"57","558",1,0);
 add_edge_graph(&g1,"58","437",1,0);
 add_edge_graph(&g1,"58","534",1,0);
 add_edge_graph(&g1,"59","466",1,0);
 add_edge_graph(&g1,"60","108",1,0);
 add_edge_graph(&g1,"60","549",1,0);
 add_edge_graph(&g1,"61","91",1,0);
 add_edge_graph(&g1,"62","310",1,0);
 add_edge_graph(&g1,"62","420",1,0);
 add_edge_graph(&g1,"62","901",1,0);
 add_edge_graph(&g1,"62","497",1,0);
 add_edge_graph(&g1,"62","466",1,0);
 add_edge_graph(&g1,"62","979",1,0);
 add_edge_graph(&g1,"62","534",1,0);
 add_edge_graph(&g1,"62","504",1,0);
 add_edge_graph(&g1,"62","508",1,0);
 add_edge_graph(&g1,"63","371",1,0);
 add_edge_graph(&g1,"63","286",1,0);
 add_edge_graph(&g1,"65","786",1,0);
 add_edge_graph(&g1,"65","371",1,0);
 add_edge_graph(&g1,"65","558",1,0);
 add_edge_graph(&g1,"66","682",1,0);
 add_edge_graph(&g1,"66","534",1,0);
 add_edge_graph(&g1,"66","727",1,0);
 add_edge_graph(&g1,"67","571",1,0);
 add_edge_graph(&g1,"68","129",1,0);
 add_edge_graph(&g1,"68","581",1,0);
 add_edge_graph(&g1,"68","545",1,0);
 add_edge_graph(&g1,"69","259",1,0);
 add_edge_graph(&g1,"69","541",1,0);
 add_edge_graph(&g1,"70","281",1,0);
 add_edge_graph(&g1,"70","979",1,0);
 add_edge_graph(&g1,"71","895",1,0);
 add_edge_graph(&g1,"72","295",1,0);
 add_edge_graph(&g1,"72","871",1,0);
 add_edge_graph(&g1,"73","891",1,0);
 add_edge_graph(&g1,"74","306",1,0);
 add_edge_graph(&g1,"74","546",1,0);
 add_edge_graph(&g1,"74","534",1,0);
 add_edge_graph(&g1,"75","979",1,0);
 add_edge_graph(&g1,"75","643",1,0);
 add_edge_graph(&g1,"75","605",1,0);
 add_edge_graph(&g1,"75","911",1,0);
 add_edge_graph(&g1,"76","737",1,0);
 add_edge_graph(&g1,"77","352",1,0);
 add_edge_graph(&g1,"78","280",1,0);
 add_edge_graph(&g1,"78","480",1,0);
 add_edge_graph(&g1,"79","114",1,0);
 add_edge_graph(&g1,"80","888",1,0);
 add_edge_graph(&g1,"80","420",1,0);
 add_edge_graph(&g1,"81","377",1,0);
 add_edge_graph(&g1,"81","371",1,0);
 add_edge_graph(&g1,"81","303",1,0);
 add_edge_graph(&g1,"82","771",1,0);
 add_edge_graph(&g1,"82","682",1,0);
 add_edge_graph(&g1,"82","720",1,0);
 add_edge_graph(&g1,"82","371",1,0);
 add_edge_graph(&g1,"82","471",1,0);
 add_edge_graph(&g1,"82","574",1,0);
 add_edge_graph(&g1,"83","277",1,0);
 add_edge_graph(&g1,"83","261",1,0);
 add_edge_graph(&g1,"84","296",1,0);
 add_edge_graph(&g1,"85","582",1,0);
 add_edge_graph(&g1,"85","682",1,0);
 add_edge_graph(&g1,"85","467",1,0);
 add_edge_graph(&g1,"85","723",1,0);
 add_edge_graph(&g1,"85","306",1,0);
 add_edge_graph(&g1,"85","534",1,0);
 add_edge_graph(&g1,"85","471",1,0);
 add_edge_graph(&g1,"85","426",1,0);
 add_edge_graph(&g1,"85","317",1,0);
 add_edge_graph(&g1,"85","101",1,0);
 add_edge_graph(&g1,"86","928",1,0);
 add_edge_graph(&g1,"86","929",1,0);
 add_edge_graph(&g1,"86","466",1,0);
 add_edge_graph(&g1,"86","522",1,0);
 add_edge_graph(&g1,"86","189",1,0);
 add_edge_graph(&g1,"87","534",1,0);
 add_edge_graph(&g1,"88","545",1,0);
 add_edge_graph(&g1,"88","436",1,0);
 add_edge_graph(&g1,"89","534",1,0);
 add_edge_graph(&g1,"90","471",1,0);
 add_edge_graph(&g1,"91","906",1,0);
 add_edge_graph(&g1,"91","786",1,0);
 add_edge_graph(&g1,"91","534",1,0);
 add_edge_graph(&g1,"91","794",1,0);
 add_edge_graph(&g1,"91","155",1,0);
 add_edge_graph(&g1,"91","282",1,0);
 add_edge_graph(&g1,"91","545",1,0);
 add_edge_graph(&g1,"91","420",1,0);
 add_edge_graph(&g1,"91","807",1,0);
 add_edge_graph(&g1,"91","808",1,0);
 add_edge_graph(&g1,"91","306",1,0);
 add_edge_graph(&g1,"91","563",1,0);
 add_edge_graph(&g1,"91","696",1,0);
 add_edge_graph(&g1,"91","198",1,0);
 add_edge_graph(&g1,"91","715",1,0);
 add_edge_graph(&g1,"91","972",1,0);
 add_edge_graph(&g1,"91","334",1,0);
 add_edge_graph(&g1,"91","466",1,0);
 add_edge_graph(&g1,"91","471",1,0);
 add_edge_graph(&g1,"91","729",1,0);
 add_edge_graph(&g1,"91","997",1,0);
 add_edge_graph(&g1,"91","494",1,0);
 add_edge_graph(&g1,"91","114",1,0);
 add_edge_graph(&g1,"91","371",1,0);
 add_edge_graph(&g1,"91","246",1,0);
 add_edge_graph(&g1,"91","123",1,0);
 add_edge_graph(&g1,"92","570",1,0);
 add_edge_graph(&g1,"92","159",1,0);
 add_edge_graph(&g1,"93","466",1,0);
 add_edge_graph(&g1,"93","371",1,0);
 add_edge_graph(&g1,"93","682",1,0);
 add_edge_graph(&g1,"93","446",1,0);
 add_edge_graph(&g1,"94","471",1,0);
 add_edge_graph(&g1,"95","569",1,0);
 add_edge_graph(&g1,"95","159",1,0);
 add_edge_graph(&g1,"96","704",1,0);
 add_edge_graph(&g1,"96","371",1,0);
 add_edge_graph(&g1,"97","471",1,0);
 add_edge_graph(&g1,"97","351",1,0);
 add_edge_graph(&g1,"98","997",1,0);
 add_edge_graph(&g1,"99","534",1,0);
 add_edge_graph(&g1,"100","444",1,0);
 add_edge_graph(&g1,"101","371",1,0);
 add_edge_graph(&g1,"101","471",1,0);
 add_edge_graph(&g1,"102","780",1,0);
 add_edge_graph(&g1,"102","977",1,0);
 add_edge_graph(&g1,"102","371",1,0);
 add_edge_graph(&g1,"102","534",1,0);
 add_edge_graph(&g1,"102","471",1,0);
 add_edge_graph(&g1,"102","538",1,0);
 add_edge_graph(&g1,"102","347",1,0);
 add_edge_graph(&g1,"103","196",1,0);
 add_edge_graph(&g1,"103","295",1,0);
 add_edge_graph(&g1,"103","371",1,0);
 add_edge_graph(&g1,"103","534",1,0);
 add_edge_graph(&g1,"103","280",1,0);
 add_edge_graph(&g1,"103","444",1,0);
 add_edge_graph(&g1,"103","351",1,0);
 add_edge_graph(&g1,"104","420",1,0);
 add_edge_graph(&g1,"104","181",1,0);
 add_edge_graph(&g1,"104","318",1,0);
 add_edge_graph(&g1,"105","312",1,0);
 add_edge_graph(&g1,"106","673",1,0);
 add_edge_graph(&g1,"106","164",1,0);
 add_edge_graph(&g1,"106","793",1,0);
 add_edge_graph(&g1,"106","449",1,0);
 add_edge_graph(&g1,"107","328",1,0);
 add_edge_graph(&g1,"107","266",1,0);
 add_edge_graph(&g1,"109","437",1,0);
 add_edge_graph(&g1,"109","914",1,0);
 add_edge_graph(&g1,"109","972",1,0);
 add_edge_graph(&g1,"109","541",1,0);
 add_edge_graph(&g1,"109","303",1,0);
 add_edge_graph(&g1,"110","504",1,0);
 add_edge_graph(&g1,"110","928",1,0);
 add_edge_graph(&g1,"110","371",1,0);
 add_edge_graph(&g1,"110","534",1,0);
 add_edge_graph(&g1,"111","322",1,0);
 add_edge_graph(&g1,"111","306",1,0);
 add_edge_graph(&g1,"111","303",1,0);
 add_edge_graph(&g1,"112","682",1,0);
 add_edge_graph(&g1,"113","523",1,0);
 add_edge_graph(&g1,"114","130",1,0);
 add_edge_graph(&g1,"114","259",1,0);
 add_edge_graph(&g1,"114","719",1,0);
 add_edge_graph(&g1,"114","909",1,0);
 add_edge_graph(&g1,"114","528",1,0);
 add_edge_graph(&g1,"114","277",1,0);
 add_edge_graph(&g1,"114","534",1,0);
 add_edge_graph(&g1,"114","281",1,0);
 add_edge_graph(&g1,"114","282",1,0);
 add_edge_graph(&g1,"114","924",1,0);
 add_edge_graph(&g1,"114","928",1,0);
 add_edge_graph(&g1,"114","420",1,0);
 add_edge_graph(&g1,"114","425",1,0);
 add_edge_graph(&g1,"114","941",1,0);
 add_edge_graph(&g1,"114","558",1,0);
 add_edge_graph(&g1,"114","560",1,0);
 add_edge_graph(&g1,"114","306",1,0);
 add_edge_graph(&g1,"114","179",1,0);
 add_edge_graph(&g1,"114","308",1,0);
 add_edge_graph(&g1,"114","437",1,0);
 add_edge_graph(&g1,"114","347",1,0);
 add_edge_graph(&g1,"114","571",1,0);
 add_edge_graph(&g1,"114","189",1,0);
 add_edge_graph(&g1,"114","574",1,0);
 add_edge_graph(&g1,"114","960",1,0);
 add_edge_graph(&g1,"114","195",1,0);
 add_edge_graph(&g1,"114","837",1,0);
 add_edge_graph(&g1,"114","327",1,0);
 add_edge_graph(&g1,"114","968",1,0);
 add_edge_graph(&g1,"114","970",1,0);
 add_edge_graph(&g1,"114","334",1,0);
 add_edge_graph(&g1,"114","466",1,0);
 add_edge_graph(&g1,"114","723",1,0);
 add_edge_graph(&g1,"114","471",1,0);
 add_edge_graph(&g1,"114","540",1,0);
 add_edge_graph(&g1,"114","480",1,0);
 add_edge_graph(&g1,"114","371",1,0);
 add_edge_graph(&g1,"114","682",1,0);
 add_edge_graph(&g1,"115","281",1,0);
 add_edge_graph(&g1,"115","371",1,0);
 add_edge_graph(&g1,"115","380",1,0);
 add_edge_graph(&g1,"115","887",1,0);
 add_edge_graph(&g1,"116","699",1,0);
 add_edge_graph(&g1,"117","997",1,0);
 add_edge_graph(&g1,"118","216",1,0);
 add_edge_graph(&g1,"118","534",1,0);
 add_edge_graph(&g1,"119","883",1,0);
 add_edge_graph(&g1,"119","371",1,0);
 add_edge_graph(&g1,"120","896",1,0);
 add_edge_graph(&g1,"120","420",1,0);
 add_edge_graph(&g1,"120","997",1,0);
 add_edge_graph(&g1,"120","583",1,0);
 add_edge_graph(&g1,"120","558",1,0);
 add_edge_graph(&g1,"120","371",1,0);
 add_edge_graph(&g1,"120","437",1,0);
 add_edge_graph(&g1,"120","694",1,0);
 add_edge_graph(&g1,"120","184",1,0);
 add_edge_graph(&g1,"120","281",1,0);
 add_edge_graph(&g1,"120","381",1,0);
 add_edge_graph(&g1,"120","831",1,0);
 add_edge_graph(&g1,"121","130",1,0);
 add_edge_graph(&g1,"121","306",1,0);
 add_edge_graph(&g1,"121","405",1,0);
 add_edge_graph(&g1,"121","410",1,0);
 add_edge_graph(&g1,"122","130",1,0);
 add_edge_graph(&g1,"123","353",1,0);
 add_edge_graph(&g1,"123","130",1,0);
 add_edge_graph(&g1,"123","452",1,0);
 add_edge_graph(&g1,"123","133",1,0);
 add_edge_graph(&g1,"123","454",1,0);
 add_edge_graph(&g1,"123","235",1,0);
 add_edge_graph(&g1,"123","371",1,0);
 add_edge_graph(&g1,"123","534",1,0);
 add_edge_graph(&g1,"124","952",1,0);
 add_edge_graph(&g1,"124","656",1,0);
 add_edge_graph(&g1,"124","186",1,0);
 add_edge_graph(&g1,"124","723",1,0);
 add_edge_graph(&g1,"124","780",1,0);
 add_edge_graph(&g1,"125","682",1,0);
 add_edge_graph(&g1,"125","371",1,0);
 add_edge_graph(&g1,"126","904",1,0);
 add_edge_graph(&g1,"126","466",1,0);
 add_edge_graph(&g1,"126","582",1,0);
 add_edge_graph(&g1,"126","374",1,0);
 add_edge_graph(&g1,"127","979",1,0);
 add_edge_graph(&g1,"127","228",1,0);
 add_edge_graph(&g1,"128","437",1,0);
 add_edge_graph(&g1,"129","704",1,0);
 add_edge_graph(&g1,"129","385",1,0);
 add_edge_graph(&g1,"129","130",1,0);
 add_edge_graph(&g1,"129","534",1,0);
 add_edge_graph(&g1,"129","582",1,0);
 add_edge_graph(&g1,"129","682",1,0);
 add_edge_graph(&g1,"129","873",1,0);
 add_edge_graph(&g1,"129","306",1,0);
 add_edge_graph(&g1,"129","371",1,0);
 add_edge_graph(&g1,"129","820",1,0);
 add_edge_graph(&g1,"129","437",1,0);
 add_edge_graph(&g1,"129","723",1,0);
 add_edge_graph(&g1,"129","471",1,0);
 add_edge_graph(&g1,"129","952",1,0);
 add_edge_graph(&g1,"129","282",1,0);
 add_edge_graph(&g1,"129","701",1,0);
 add_edge_graph(&g1,"130","514",1,0);
 add_edge_graph(&g1,"130","516",1,0);
 add_edge_graph(&g1,"130","522",1,0);
 add_edge_graph(&g1,"130","525",1,0);
 add_edge_graph(&g1,"130","534",1,0);
 add_edge_graph(&g1,"130","544",1,0);
 add_edge_graph(&g1,"130","571",1,0);
 add_edge_graph(&g1,"130","572",1,0);
 add_edge_graph(&g1,"130","574",1,0);
 add_edge_graph(&g1,"130","577",1,0);
 add_edge_graph(&g1,"130","582",1,0);
 add_edge_graph(&g1,"130","587",1,0);
 add_edge_graph(&g1,"130","597",1,0);
 add_edge_graph(&g1,"130","604",1,0);
 add_edge_graph(&g1,"130","605",1,0);
 add_edge_graph(&g1,"130","611",1,0);
 add_edge_graph(&g1,"130","612",1,0);
 add_edge_graph(&g1,"130","619",1,0);
 add_edge_graph(&g1,"130","628",1,0);
 add_edge_graph(&g1,"130","135",1,0);
 add_edge_graph(&g1,"130","137",1,0);
 add_edge_graph(&g1,"130","148",1,0);
 add_edge_graph(&g1,"130","151",1,0);
 add_edge_graph(&g1,"130","667",1,0);
 add_edge_graph(&g1,"130","669",1,0);
 add_edge_graph(&g1,"130","159",1,0);
 add_edge_graph(&g1,"130","193",1,0);
 add_edge_graph(&g1,"130","174",1,0);
 add_edge_graph(&g1,"130","687",1,0);
 add_edge_graph(&g1,"130","176",1,0);
 add_edge_graph(&g1,"130","705",1,0);
 add_edge_graph(&g1,"130","709",1,0);
 add_edge_graph(&g1,"130","545",1,0);
 add_edge_graph(&g1,"130","715",1,0);
 add_edge_graph(&g1,"130","206",1,0);
 add_edge_graph(&g1,"130","209",1,0);
 add_edge_graph(&g1,"130","723",1,0);
 add_edge_graph(&g1,"130","220",1,0);
 add_edge_graph(&g1,"130","224",1,0);
 add_edge_graph(&g1,"130","230",1,0);
 add_edge_graph(&g1,"130","234",1,0);
 add_edge_graph(&g1,"130","235",1,0);
 add_edge_graph(&g1,"130","466",1,0);
 add_edge_graph(&g1,"130","754",1,0);
 add_edge_graph(&g1,"130","246",1,0);
 add_edge_graph(&g1,"130","768",1,0);
 add_edge_graph(&g1,"130","770",1,0);
 add_edge_graph(&g1,"130","641",1,0);
 add_edge_graph(&g1,"130","277",1,0);
 add_edge_graph(&g1,"130","278",1,0);
 add_edge_graph(&g1,"130","280",1,0);
 add_edge_graph(&g1,"130","281",1,0);
 add_edge_graph(&g1,"130","288",1,0);
 add_edge_graph(&g1,"130","294",1,0);
 add_edge_graph(&g1,"130","295",1,0);
 add_edge_graph(&g1,"130","809",1,0);
 add_edge_graph(&g1,"130","810",1,0);
 add_edge_graph(&g1,"130","299",1,0);
 add_edge_graph(&g1,"130","303",1,0);
 add_edge_graph(&g1,"130","304",1,0);
 add_edge_graph(&g1,"130","306",1,0);
 add_edge_graph(&g1,"130","310",1,0);
 add_edge_graph(&g1,"130","324",1,0);
 add_edge_graph(&g1,"130","822",1,0);
 add_edge_graph(&g1,"130","327",1,0);
 add_edge_graph(&g1,"130","328",1,0);
 add_edge_graph(&g1,"130","338",1,0);
 add_edge_graph(&g1,"130","853",1,0);
 add_edge_graph(&g1,"130","859",1,0);
 add_edge_graph(&g1,"130","349",1,0);
 add_edge_graph(&g1,"130","351",1,0);
 add_edge_graph(&g1,"130","869",1,0);
 add_edge_graph(&g1,"130","871",1,0);
 add_edge_graph(&g1,"130","875",1,0);
 add_edge_graph(&g1,"130","878",1,0);
 add_edge_graph(&g1,"130","371",1,0);
 add_edge_graph(&g1,"130","374",1,0);
 add_edge_graph(&g1,"130","375",1,0);
 add_edge_graph(&g1,"130","385",1,0);
 add_edge_graph(&g1,"130","390",1,0);
 add_edge_graph(&g1,"130","395",1,0);
 add_edge_graph(&g1,"130","400",1,0);
 add_edge_graph(&g1,"130","915",1,0);
 add_edge_graph(&g1,"130","405",1,0);
 add_edge_graph(&g1,"130","410",1,0);
 add_edge_graph(&g1,"130","926",1,0);
 add_edge_graph(&g1,"130","837",1,0);
 add_edge_graph(&g1,"130","928",1,0);
 add_edge_graph(&g1,"130","929",1,0);
 add_edge_graph(&g1,"130","420",1,0);
 add_edge_graph(&g1,"130","933",1,0);
 add_edge_graph(&g1,"130","428",1,0);
 add_edge_graph(&g1,"130","430",1,0);
 add_edge_graph(&g1,"130","948",1,0);
 add_edge_graph(&g1,"130","437",1,0);
 add_edge_graph(&g1,"130","444",1,0);
 add_edge_graph(&g1,"130","446",1,0);
 add_edge_graph(&g1,"130","964",1,0);
 add_edge_graph(&g1,"130","966",1,0);
 add_edge_graph(&g1,"130","972",1,0);
 add_edge_graph(&g1,"130","462",1,0);
 add_edge_graph(&g1,"130","978",1,0);
 add_edge_graph(&g1,"130","471",1,0);
 add_edge_graph(&g1,"130","987",1,0);
 add_edge_graph(&g1,"130","476",1,0);
 add_edge_graph(&g1,"130","992",1,0);
 add_edge_graph(&g1,"130","994",1,0);
 add_edge_graph(&g1,"130","484",1,0);
 add_edge_graph(&g1,"130","997",1,0);
 add_edge_graph(&g1,"130","490",1,0);
 add_edge_graph(&g1,"130","496",1,0);
 add_edge_graph(&g1,"130","506",1,0);
 add_edge_graph(&g1,"130","682",1,0);
 add_edge_graph(&g1,"130","510",1,0);
 add_edge_graph(&g1,"131","515",1,0);
 add_edge_graph(&g1,"131","936",1,0);
 add_edge_graph(&g1,"131","553",1,0);
 add_edge_graph(&g1,"131","371",1,0);
 add_edge_graph(&g1,"131","437",1,0);
 add_edge_graph(&g1,"131","574",1,0);
 add_edge_graph(&g1,"132","720",1,0);
 add_edge_graph(&g1,"132","474",1,0);
 add_edge_graph(&g1,"133","586",1,0);
 add_edge_graph(&g1,"133","371",1,0);
 add_edge_graph(&g1,"133","471",1,0);
 add_edge_graph(&g1,"133","344",1,0);
 add_edge_graph(&g1,"133","490",1,0);
 add_edge_graph(&g1,"134","281",1,0);
 add_edge_graph(&g1,"134","266",1,0);
 add_edge_graph(&g1,"136","582",1,0);
 add_edge_graph(&g1,"137","793",1,0);
 add_edge_graph(&g1,"138","532",1,0);
 add_edge_graph(&g1,"138","597",1,0);
 add_edge_graph(&g1,"138","758",1,0);
 add_edge_graph(&g1,"139","310",1,0);
 add_edge_graph(&g1,"140","437",1,0);
 add_edge_graph(&g1,"141","694",1,0);
 add_edge_graph(&g1,"142","489",1,0);
 add_edge_graph(&g1,"142","559",1,0);
 add_edge_graph(&g1,"142","466",1,0);
 add_edge_graph(&g1,"142","371",1,0);
 add_edge_graph(&g1,"142","534",1,0);
 add_edge_graph(&g1,"143","306",1,0);
 add_edge_graph(&g1,"143","371",1,0);
 add_edge_graph(&g1,"143","246",1,0);
 add_edge_graph(&g1,"144","594",1,0);
 add_edge_graph(&g1,"144","371",1,0);
 add_edge_graph(&g1,"145","371",1,0);
 add_edge_graph(&g1,"146","819",1,0);
 add_edge_graph(&g1,"147","605",1,0);
 add_edge_graph(&g1,"149","590",1,0);
 add_edge_graph(&g1,"150","306",1,0);
 add_edge_graph(&g1,"150","534",1,0);
 add_edge_graph(&g1,"150","727",1,0);
 add_edge_graph(&g1,"151","682",1,0);
 add_edge_graph(&g1,"151","270",1,0);
 add_edge_graph(&g1,"152","939",1,0);
 add_edge_graph(&g1,"153","432",1,0);
 add_edge_graph(&g1,"153","593",1,0);
 add_edge_graph(&g1,"153","605",1,0);
 add_edge_graph(&g1,"154","226",1,0);
 add_edge_graph(&g1,"154","466",1,0);
 add_edge_graph(&g1,"155","281",1,0);
 add_edge_graph(&g1,"155","466",1,0);
 add_edge_graph(&g1,"155","746",1,0);
 add_edge_graph(&g1,"155","534",1,0);
 add_edge_graph(&g1,"156","504",1,0);
 add_edge_graph(&g1,"157","371",1,0);
 add_edge_graph(&g1,"158","545",1,0);
 add_edge_graph(&g1,"159","246",1,0);
 add_edge_graph(&g1,"159","277",1,0);
 add_edge_graph(&g1,"159","662",1,0);
 add_edge_graph(&g1,"159","582",1,0);
 add_edge_graph(&g1,"159","682",1,0);
 add_edge_graph(&g1,"159","936",1,0);
 add_edge_graph(&g1,"159","829",1,0);
 add_edge_graph(&g1,"159","371",1,0);
 add_edge_graph(&g1,"159","756",1,0);
 add_edge_graph(&g1,"159","405",1,0);
 add_edge_graph(&g1,"159","534",1,0);
 add_edge_graph(&g1,"159","471",1,0);
 add_edge_graph(&g1,"159","633",1,0);
 add_edge_graph(&g1,"159","789",1,0);
 add_edge_graph(&g1,"159","303",1,0);
 add_edge_graph(&g1,"159","778",1,0);
 add_edge_graph(&g1,"160","306",1,0);
 add_edge_graph(&g1,"160","822",1,0);
 add_edge_graph(&g1,"160","582",1,0);
 add_edge_graph(&g1,"160","437",1,0);
 add_edge_graph(&g1,"161","371",1,0);
 add_edge_graph(&g1,"162","303",1,0);
 add_edge_graph(&g1,"162","574",1,0);
 add_edge_graph(&g1,"162","837",1,0);
 add_edge_graph(&g1,"162","806",1,0);
 add_edge_graph(&g1,"162","471",1,0);
 add_edge_graph(&g1,"163","371",1,0);
 add_edge_graph(&g1,"164","303",1,0);
 add_edge_graph(&g1,"165","994",1,0);
 add_edge_graph(&g1,"165","908",1,0);
 add_edge_graph(&g1,"165","471",1,0);
 add_edge_graph(&g1,"166","371",1,0);
 add_edge_graph(&g1,"167","523",1,0);
 add_edge_graph(&g1,"168","970",1,0);
 add_edge_graph(&g1,"169","682",1,0);
 add_edge_graph(&g1,"170","629",1,0);
 add_edge_graph(&g1,"170","715",1,0);
 add_edge_graph(&g1,"170","412",1,0);
 add_edge_graph(&g1,"170","957",1,0);
 add_edge_graph(&g1,"170","471",1,0);
 add_edge_graph(&g1,"171","930",1,0);
 add_edge_graph(&g1,"172","575",1,0);
 add_edge_graph(&g1,"172","324",1,0);
 add_edge_graph(&g1,"172","839",1,0);
 add_edge_graph(&g1,"173","281",1,0);
 add_edge_graph(&g1,"173","378",1,0);
 add_edge_graph(&g1,"174","667",1,0);
 add_edge_graph(&g1,"175","202",1,0);
 add_edge_graph(&g1,"175","723",1,0);
 add_edge_graph(&g1,"175","428",1,0);
 add_edge_graph(&g1,"177","768",1,0);
 add_edge_graph(&g1,"177","705",1,0);
 add_edge_graph(&g1,"177","270",1,0);
 add_edge_graph(&g1,"177","371",1,0);
 add_edge_graph(&g1,"177","281",1,0);
 add_edge_graph(&g1,"177","605",1,0);
 add_edge_graph(&g1,"179","859",1,0);
 add_edge_graph(&g1,"179","682",1,0);
 add_edge_graph(&g1,"179","471",1,0);
 add_edge_graph(&g1,"180","306",1,0);
 add_edge_graph(&g1,"181","605",1,0);
 add_edge_graph(&g1,"182","371",1,0);
 add_edge_graph(&g1,"183","916",1,0);
 add_edge_graph(&g1,"183","437",1,0);
 add_edge_graph(&g1,"184","371",1,0);
 add_edge_graph(&g1,"184","534",1,0);
 add_edge_graph(&g1,"185","280",1,0);
 add_edge_graph(&g1,"185","306",1,0);
 add_edge_graph(&g1,"185","371",1,0);
 add_edge_graph(&g1,"187","574",1,0);
 add_edge_graph(&g1,"188","534",1,0);
 add_edge_graph(&g1,"188","718",1,0);
 add_edge_graph(&g1,"190","937",1,0);
 add_edge_graph(&g1,"190","381",1,0);
 add_edge_graph(&g1,"191","864",1,0);
 add_edge_graph(&g1,"191","371",1,0);
 add_edge_graph(&g1,"191","643",1,0);
 add_edge_graph(&g1,"192","305",1,0);
 add_edge_graph(&g1,"192","701",1,0);
 add_edge_graph(&g1,"192","582",1,0);
 add_edge_graph(&g1,"192","471",1,0);
 add_edge_graph(&g1,"193","997",1,0);
 add_edge_graph(&g1,"194","371",1,0);
 add_edge_graph(&g1,"194","427",1,0);
 add_edge_graph(&g1,"194","582",1,0);
 add_edge_graph(&g1,"195","563",1,0);
 add_edge_graph(&g1,"196","979",1,0);
 add_edge_graph(&g1,"197","926",1,0);
 add_edge_graph(&g1,"198","827",1,0);
 add_edge_graph(&g1,"198","266",1,0);
 add_edge_graph(&g1,"198","534",1,0);
 add_edge_graph(&g1,"198","282",1,0);
 add_edge_graph(&g1,"199","306",1,0);
 add_edge_graph(&g1,"200","466",1,0);
 add_edge_graph(&g1,"200","437",1,0);
 add_edge_graph(&g1,"201","723",1,0);
 add_edge_graph(&g1,"201","357",1,0);
 add_edge_graph(&g1,"201","471",1,0);
 add_edge_graph(&g1,"202","519",1,0);
 add_edge_graph(&g1,"203","723",1,0);
 add_edge_graph(&g1,"204","449",1,0);
 add_edge_graph(&g1,"204","793",1,0);
 add_edge_graph(&g1,"205","371",1,0);
 add_edge_graph(&g1,"207","466",1,0);
 add_edge_graph(&g1,"207","844",1,0);
 add_edge_graph(&g1,"208","723",1,0);
 add_edge_graph(&g1,"208","499",1,0);
 add_edge_graph(&g1,"209","601",1,0);
 add_edge_graph(&g1,"209","471",1,0);
 add_edge_graph(&g1,"210","275",1,0);
 add_edge_graph(&g1,"211","928",1,0);
 add_edge_graph(&g1,"212","282",1,0);
 add_edge_graph(&g1,"212","371",1,0);
 add_edge_graph(&g1,"213","612",1,0);
 add_edge_graph(&g1,"213","471",1,0);
 add_edge_graph(&g1,"214","330",1,0);
 add_edge_graph(&g1,"215","306",1,0);
 add_edge_graph(&g1,"215","371",1,0);
 add_edge_graph(&g1,"215","701",1,0);
 add_edge_graph(&g1,"215","334",1,0);
 add_edge_graph(&g1,"216","344",1,0);
 add_edge_graph(&g1,"216","492",1,0);
 add_edge_graph(&g1,"217","261",1,0);
 add_edge_graph(&g1,"217","582",1,0);
 add_edge_graph(&g1,"217","619",1,0);
 add_edge_graph(&g1,"217","303",1,0);
 add_edge_graph(&g1,"217","595",1,0);
 add_edge_graph(&g1,"217","471",1,0);
 add_edge_graph(&g1,"217","605",1,0);
 add_edge_graph(&g1,"217","574",1,0);
 add_edge_graph(&g1,"218","374",1,0);
 add_edge_graph(&g1,"219","849",1,0);
 add_edge_graph(&g1,"219","878",1,0);
 add_edge_graph(&g1,"220","569",1,0);
 add_edge_graph(&g1,"220","371",1,0);
 add_edge_graph(&g1,"221","371",1,0);
 add_edge_graph(&g1,"222","849",1,0);
 add_edge_graph(&g1,"223","371",1,0);
 add_edge_graph(&g1,"223","682",1,0);
 add_edge_graph(&g1,"223","819",1,0);
 add_edge_graph(&g1,"223","405",1,0);
 add_edge_graph(&g1,"223","782",1,0);
 add_edge_graph(&g1,"224","704",1,0);
 add_edge_graph(&g1,"224","389",1,0);
 add_edge_graph(&g1,"224","277",1,0);
 add_edge_graph(&g1,"225","456",1,0);
 add_edge_graph(&g1,"225","929",1,0);
 add_edge_graph(&g1,"225","371",1,0);
 add_edge_graph(&g1,"226","582",1,0);
 add_edge_graph(&g1,"226","935",1,0);
 add_edge_graph(&g1,"226","936",1,0);
 add_edge_graph(&g1,"226","306",1,0);
 add_edge_graph(&g1,"226","466",1,0);
 add_edge_graph(&g1,"226","371",1,0);
 add_edge_graph(&g1,"226","692",1,0);
 add_edge_graph(&g1,"226","723",1,0);
 add_edge_graph(&g1,"226","471",1,0);
 add_edge_graph(&g1,"226","984",1,0);
 add_edge_graph(&g1,"226","793",1,0);
 add_edge_graph(&g1,"226","574",1,0);
 add_edge_graph(&g1,"227","351",1,0);
 add_edge_graph(&g1,"228","449",1,0);
 add_edge_graph(&g1,"228","997",1,0);
 add_edge_graph(&g1,"228","717",1,0);
 add_edge_graph(&g1,"228","429",1,0);
 add_edge_graph(&g1,"228","371",1,0);
 add_edge_graph(&g1,"228","791",1,0);
 add_edge_graph(&g1,"228","506",1,0);
 add_edge_graph(&g1,"229","449",1,0);
 add_edge_graph(&g1,"229","306",1,0);
 add_edge_graph(&g1,"229","371",1,0);
 add_edge_graph(&g1,"229","428",1,0);
 add_edge_graph(&g1,"229","439",1,0);
 add_edge_graph(&g1,"230","650",1,0);
 add_edge_graph(&g1,"230","471",1,0);
 add_edge_graph(&g1,"231","282",1,0);
 add_edge_graph(&g1,"232","682",1,0);
 add_edge_graph(&g1,"233","405",1,0);
 add_edge_graph(&g1,"234","334",1,0);
 add_edge_graph(&g1,"234","559",1,0);
 add_edge_graph(&g1,"234","656",1,0);
 add_edge_graph(&g1,"234","405",1,0);
 add_edge_graph(&g1,"234","471",1,0);
 add_edge_graph(&g1,"234","926",1,0);
 add_edge_graph(&g1,"235","928",1,0);
 add_edge_graph(&g1,"235","545",1,0);
 add_edge_graph(&g1,"235","328",1,0);
 add_edge_graph(&g1,"235","329",1,0);
 add_edge_graph(&g1,"235","471",1,0);
 add_edge_graph(&g1,"235","877",1,0);
 add_edge_graph(&g1,"235","430",1,0);
 add_edge_graph(&g1,"235","418",1,0);
 add_edge_graph(&g1,"235","466",1,0);
 add_edge_graph(&g1,"235","371",1,0);
 add_edge_graph(&g1,"235","597",1,0);
 add_edge_graph(&g1,"235","534",1,0);
 add_edge_graph(&g1,"235","462",1,0);
 add_edge_graph(&g1,"235","602",1,0);
 add_edge_graph(&g1,"235","405",1,0);
 add_edge_graph(&g1,"236","280",1,0);
 add_edge_graph(&g1,"236","282",1,0);
 add_edge_graph(&g1,"236","787",1,0);
 add_edge_graph(&g1,"237","720",1,0);
 add_edge_graph(&g1,"237","381",1,0);
 add_edge_graph(&g1,"238","534",1,0);
 add_edge_graph(&g1,"238","982",1,0);
 add_edge_graph(&g1,"238","775",1,0);
 add_edge_graph(&g1,"239","555",1,0);
 add_edge_graph(&g1,"240","281",1,0);
 add_edge_graph(&g1,"241","793",1,0);
 add_edge_graph(&g1,"241","294",1,0);
 add_edge_graph(&g1,"241","303",1,0);
 add_edge_graph(&g1,"242","420",1,0);
 add_edge_graph(&g1,"243","471",1,0);
 add_edge_graph(&g1,"244","682",1,0);
 add_edge_graph(&g1,"246","608",1,0);
 add_edge_graph(&g1,"246","388",1,0);
 add_edge_graph(&g1,"246","295",1,0);
 add_edge_graph(&g1,"246","905",1,0);
 add_edge_graph(&g1,"246","970",1,0);
 add_edge_graph(&g1,"246","371",1,0);
 add_edge_graph(&g1,"246","404",1,0);
 add_edge_graph(&g1,"246","534",1,0);
 add_edge_graph(&g1,"246","471",1,0);
 add_edge_graph(&g1,"246","655",1,0);
 add_edge_graph(&g1,"247","720",1,0);
 add_edge_graph(&g1,"248","327",1,0);
 add_edge_graph(&g1,"249","785",1,0);
 add_edge_graph(&g1,"249","371",1,0);
 add_edge_graph(&g1,"250","281",1,0);
 add_edge_graph(&g1,"250","282",1,0);
 add_edge_graph(&g1,"251","843",1,0);
 add_edge_graph(&g1,"251","534",1,0);
 add_edge_graph(&g1,"252","405",1,0);
 add_edge_graph(&g1,"253","371",1,0);
 add_edge_graph(&g1,"253","798",1,0);
 add_edge_graph(&g1,"254","406",1,0);
 add_edge_graph(&g1,"255","281",1,0);
 add_edge_graph(&g1,"255","471",1,0);
 add_edge_graph(&g1,"256","504",1,0);
 add_edge_graph(&g1,"257","471",1,0);
 add_edge_graph(&g1,"258","471",1,0);
 add_edge_graph(&g1,"259","419",1,0);
 add_edge_graph(&g1,"259","683",1,0);
 add_edge_graph(&g1,"259","466",1,0);
 add_edge_graph(&g1,"259","303",1,0);
 add_edge_graph(&g1,"259","753",1,0);
 add_edge_graph(&g1,"259","371",1,0);
 add_edge_graph(&g1,"259","306",1,0);
 add_edge_graph(&g1,"259","410",1,0);
 add_edge_graph(&g1,"259","444",1,0);
 add_edge_graph(&g1,"259","605",1,0);
 add_edge_graph(&g1,"259","542",1,0);
 add_edge_graph(&g1,"260","306",1,0);
 add_edge_graph(&g1,"260","371",1,0);
 add_edge_graph(&g1,"261","292",1,0);
 add_edge_graph(&g1,"261","521",1,0);
 add_edge_graph(&g1,"261","593",1,0);
 add_edge_graph(&g1,"261","371",1,0);
 add_edge_graph(&g1,"261","917",1,0);
 add_edge_graph(&g1,"261","282",1,0);
 add_edge_graph(&g1,"261","571",1,0);
 add_edge_graph(&g1,"261","381",1,0);
 add_edge_graph(&g1,"262","571",1,0);
 add_edge_graph(&g1,"263","371",1,0);
 add_edge_graph(&g1,"263","471",1,0);
 add_edge_graph(&g1,"264","921",1,0);
 add_edge_graph(&g1,"265","612",1,0);
 add_edge_graph(&g1,"266","928",1,0);
 add_edge_graph(&g1,"266","547",1,0);
 add_edge_graph(&g1,"266","268",1,0);
 add_edge_graph(&g1,"266","371",1,0);
 add_edge_graph(&g1,"266","982",1,0);
 add_edge_graph(&g1,"266","471",1,0);
 add_edge_graph(&g1,"267","723",1,0);
 add_edge_graph(&g1,"267","659",1,0);
 add_edge_graph(&g1,"267","582",1,0);
 add_edge_graph(&g1,"268","371",1,0);
 add_edge_graph(&g1,"269","437",1,0);
 add_edge_graph(&g1,"269","471",1,0);
 add_edge_graph(&g1,"270","466",1,0);
 add_edge_graph(&g1,"270","471",1,0);
 add_edge_graph(&g1,"270","605",1,0);
 add_edge_graph(&g1,"271","281",1,0);
 add_edge_graph(&g1,"272","485",1,0);
 add_edge_graph(&g1,"273","371",1,0);
 add_edge_graph(&g1,"274","298",1,0);
 add_edge_graph(&g1,"274","846",1,0);
 add_edge_graph(&g1,"275","420",1,0);
 add_edge_graph(&g1,"275","471",1,0);
 add_edge_graph(&g1,"276","335",1,0);
 add_edge_graph(&g1,"277","404",1,0);
 add_edge_graph(&g1,"277","534",1,0);
 add_edge_graph(&g1,"277","800",1,0);
 add_edge_graph(&g1,"277","545",1,0);
 add_edge_graph(&g1,"277","303",1,0);
 add_edge_graph(&g1,"277","306",1,0);
 add_edge_graph(&g1,"277","574",1,0);
 add_edge_graph(&g1,"277","320",1,0);
 add_edge_graph(&g1,"277","964",1,0);
 add_edge_graph(&g1,"277","709",1,0);
 add_edge_graph(&g1,"277","464",1,0);
 add_edge_graph(&g1,"277","466",1,0);
 add_edge_graph(&g1,"277","471",1,0);
 add_edge_graph(&g1,"277","603",1,0);
 add_edge_graph(&g1,"277","989",1,0);
 add_edge_graph(&g1,"277","979",1,0);
 add_edge_graph(&g1,"277","612",1,0);
 add_edge_graph(&g1,"277","997",1,0);
 add_edge_graph(&g1,"277","625",1,0);
 add_edge_graph(&g1,"277","371",1,0);
 add_edge_graph(&g1,"279","420",1,0);
 add_edge_graph(&g1,"279","893",1,0);
 add_edge_graph(&g1,"280","790",1,0);
 add_edge_graph(&g1,"280","876",1,0);
 add_edge_graph(&g1,"280","525",1,0);
 add_edge_graph(&g1,"280","534",1,0);
 add_edge_graph(&g1,"280","791",1,0);
 add_edge_graph(&g1,"280","922",1,0);
 add_edge_graph(&g1,"280","926",1,0);
 add_edge_graph(&g1,"280","928",1,0);
 add_edge_graph(&g1,"280","548",1,0);
 add_edge_graph(&g1,"280","425",1,0);
 add_edge_graph(&g1,"280","682",1,0);
 add_edge_graph(&g1,"280","300",1,0);
 add_edge_graph(&g1,"280","303",1,0);
 add_edge_graph(&g1,"280","306",1,0);
 add_edge_graph(&g1,"280","692",1,0);
 add_edge_graph(&g1,"280","437",1,0);
 add_edge_graph(&g1,"280","582",1,0);
 add_edge_graph(&g1,"280","840",1,0);
 add_edge_graph(&g1,"280","466",1,0);
 add_edge_graph(&g1,"280","597",1,0);
 add_edge_graph(&g1,"280","471",1,0);
 add_edge_graph(&g1,"280","787",1,0);
 add_edge_graph(&g1,"280","605",1,0);
 add_edge_graph(&g1,"280","997",1,0);
 add_edge_graph(&g1,"280","619",1,0);
 add_edge_graph(&g1,"280","496",1,0);
 add_edge_graph(&g1,"280","497",1,0);
 add_edge_graph(&g1,"280","371",1,0);
 add_edge_graph(&g1,"280","506",1,0);
 add_edge_graph(&g1,"280","763",1,0);
 add_edge_graph(&g1,"280","637",1,0);
 add_edge_graph(&g1,"281","768",1,0);
 add_edge_graph(&g1,"281","644",1,0);
 add_edge_graph(&g1,"281","645",1,0);
 add_edge_graph(&g1,"281","405",1,0);
 add_edge_graph(&g1,"281","534",1,0);
 add_edge_graph(&g1,"281","282",1,0);
 add_edge_graph(&g1,"281","928",1,0);
 add_edge_graph(&g1,"281","418",1,0);
 add_edge_graph(&g1,"281","582",1,0);
 add_edge_graph(&g1,"281","934",1,0);
 add_edge_graph(&g1,"281","807",1,0);
 add_edge_graph(&g1,"281","680",1,0);
 add_edge_graph(&g1,"281","297",1,0);
 add_edge_graph(&g1,"281","558",1,0);
 add_edge_graph(&g1,"281","306",1,0);
 add_edge_graph(&g1,"281","371",1,0);
 add_edge_graph(&g1,"281","692",1,0);
 add_edge_graph(&g1,"281","437",1,0);
 add_edge_graph(&g1,"281","439",1,0);
 add_edge_graph(&g1,"281","952",1,0);
 add_edge_graph(&g1,"281","324",1,0);
 add_edge_graph(&g1,"281","584",1,0);
 add_edge_graph(&g1,"281","843",1,0);
 add_edge_graph(&g1,"281","972",1,0);
 add_edge_graph(&g1,"281","802",1,0);
 add_edge_graph(&g1,"281","462",1,0);
 add_edge_graph(&g1,"281","336",1,0);
 add_edge_graph(&g1,"281","504",1,0);
 add_edge_graph(&g1,"281","466",1,0);
 add_edge_graph(&g1,"281","723",1,0);
 add_edge_graph(&g1,"281","471",1,0);
 add_edge_graph(&g1,"281","728",1,0);
 add_edge_graph(&g1,"281","351",1,0);
 add_edge_graph(&g1,"281","865",1,0);
 add_edge_graph(&g1,"281","994",1,0);
 add_edge_graph(&g1,"281","995",1,0);
 add_edge_graph(&g1,"281","872",1,0);
 add_edge_graph(&g1,"281","338",1,0);
 add_edge_graph(&g1,"281","744",1,0);
 add_edge_graph(&g1,"281","937",1,0);
 add_edge_graph(&g1,"281","510",1,0);
 add_edge_graph(&g1,"282","386",1,0);
 add_edge_graph(&g1,"282","524",1,0);
 add_edge_graph(&g1,"282","417",1,0);
 add_edge_graph(&g1,"282","582",1,0);
 add_edge_graph(&g1,"282","297",1,0);
 add_edge_graph(&g1,"282","427",1,0);
 add_edge_graph(&g1,"282","813",1,0);
 add_edge_graph(&g1,"282","303",1,0);
 add_edge_graph(&g1,"282","306",1,0);
 add_edge_graph(&g1,"282","314",1,0);
 add_edge_graph(&g1,"282","571",1,0);
 add_edge_graph(&g1,"282","449",1,0);
 add_edge_graph(&g1,"282","836",1,0);
 add_edge_graph(&g1,"282","837",1,0);
 add_edge_graph(&g1,"282","466",1,0);
 add_edge_graph(&g1,"282","471",1,0);
 add_edge_graph(&g1,"282","605",1,0);
 add_edge_graph(&g1,"282","351",1,0);
 add_edge_graph(&g1,"282","480",1,0);
 add_edge_graph(&g1,"282","497",1,0);
 add_edge_graph(&g1,"282","371",1,0);
 add_edge_graph(&g1,"283","371",1,0);
 add_edge_graph(&g1,"283","940",1,0);
 add_edge_graph(&g1,"283","709",1,0);
 add_edge_graph(&g1,"284","371",1,0);
 add_edge_graph(&g1,"285","768",1,0);
 add_edge_graph(&g1,"285","354",1,0);
 add_edge_graph(&g1,"285","466",1,0);
 add_edge_graph(&g1,"285","534",1,0);
 add_edge_graph(&g1,"285","303",1,0);
 add_edge_graph(&g1,"286","334",1,0);
 add_edge_graph(&g1,"287","605",1,0);
 add_edge_graph(&g1,"288","311",1,0);
 add_edge_graph(&g1,"288","308",1,0);
 add_edge_graph(&g1,"288","818",1,0);
 add_edge_graph(&g1,"288","471",1,0);
 add_edge_graph(&g1,"289","371",1,0);
 add_edge_graph(&g1,"290","428",1,0);
 add_edge_graph(&g1,"292","437",1,0);
 add_edge_graph(&g1,"293","504",1,0);
 add_edge_graph(&g1,"293","466",1,0);
 add_edge_graph(&g1,"295","737",1,0);
 add_edge_graph(&g1,"295","499",1,0);
 add_edge_graph(&g1,"295","471",1,0);
 add_edge_graph(&g1,"295","574",1,0);
 add_edge_graph(&g1,"295","767",1,0);
 add_edge_graph(&g1,"296","791",1,0);
 add_edge_graph(&g1,"296","303",1,0);
 add_edge_graph(&g1,"297","805",1,0);
 add_edge_graph(&g1,"297","534",1,0);
 add_edge_graph(&g1,"297","972",1,0);
 add_edge_graph(&g1,"297","878",1,0);
 add_edge_graph(&g1,"297","687",1,0);
 add_edge_graph(&g1,"297","371",1,0);
 add_edge_graph(&g1,"297","723",1,0);
 add_edge_graph(&g1,"297","605",1,0);
 add_edge_graph(&g1,"299","471",1,0);
 add_edge_graph(&g1,"300","371",1,0);
 add_edge_graph(&g1,"300","303",1,0);
 add_edge_graph(&g1,"301","537",1,0);
 add_edge_graph(&g1,"301","534",1,0);
 add_edge_graph(&g1,"302","348",1,0);
 add_edge_graph(&g1,"302","574",1,0);
 add_edge_graph(&g1,"303","406",1,0);
 add_edge_graph(&g1,"303","522",1,0);
 add_edge_graph(&g1,"303","810",1,0);
 add_edge_graph(&g1,"303","895",1,0);
 add_edge_graph(&g1,"303","534",1,0);
 add_edge_graph(&g1,"303","928",1,0);
 add_edge_graph(&g1,"303","306",1,0);
 add_edge_graph(&g1,"303","437",1,0);
 add_edge_graph(&g1,"303","572",1,0);
 add_edge_graph(&g1,"303","574",1,0);
 add_edge_graph(&g1,"303","885",1,0);
 add_edge_graph(&g1,"303","832",1,0);
 add_edge_graph(&g1,"303","449",1,0);
 add_edge_graph(&g1,"303","580",1,0);
 add_edge_graph(&g1,"303","837",1,0);
 add_edge_graph(&g1,"303","968",1,0);
 add_edge_graph(&g1,"303","972",1,0);
 add_edge_graph(&g1,"303","334",1,0);
 add_edge_graph(&g1,"303","464",1,0);
 add_edge_graph(&g1,"303","466",1,0);
 add_edge_graph(&g1,"303","471",1,0);
 add_edge_graph(&g1,"303","605",1,0);
 add_edge_graph(&g1,"303","351",1,0);
 add_edge_graph(&g1,"303","998",1,0);
 add_edge_graph(&g1,"303","871",1,0);
 add_edge_graph(&g1,"303","371",1,0);
 add_edge_graph(&g1,"303","372",1,0);
 add_edge_graph(&g1,"303","373",1,0);
 add_edge_graph(&g1,"303","374",1,0);
 add_edge_graph(&g1,"303","511",1,0);
 add_edge_graph(&g1,"303","507",1,0);
 add_edge_graph(&g1,"303","682",1,0);
 add_edge_graph(&g1,"303","405",1,0);
 add_edge_graph(&g1,"304","997",1,0);
 add_edge_graph(&g1,"304","777",1,0);
 add_edge_graph(&g1,"304","621",1,0);
 add_edge_graph(&g1,"304","371",1,0);
 add_edge_graph(&g1,"304","406",1,0);
 add_edge_graph(&g1,"304","471",1,0);
 add_edge_graph(&g1,"304","926",1,0);
 add_edge_graph(&g1,"306","515",1,0);
 add_edge_graph(&g1,"306","428",1,0);
 add_edge_graph(&g1,"306","534",1,0);
 add_edge_graph(&g1,"306","558",1,0);
 add_edge_graph(&g1,"306","565",1,0);
 add_edge_graph(&g1,"306","522",1,0);
 add_edge_graph(&g1,"306","574",1,0);
 add_edge_graph(&g1,"306","582",1,0);
 add_edge_graph(&g1,"306","597",1,0);
 add_edge_graph(&g1,"306","594",1,0);
 add_edge_graph(&g1,"306","605",1,0);
 add_edge_graph(&g1,"306","608",1,0);
 add_edge_graph(&g1,"306","612",1,0);
 add_edge_graph(&g1,"306","614",1,0);
 add_edge_graph(&g1,"306","621",1,0);
 add_edge_graph(&g1,"306","623",1,0);
 add_edge_graph(&g1,"306","624",1,0);
 add_edge_graph(&g1,"306","627",1,0);
 add_edge_graph(&g1,"306","638",1,0);
 add_edge_graph(&g1,"306","682",1,0);
 add_edge_graph(&g1,"306","688",1,0);
 add_edge_graph(&g1,"306","705",1,0);
 add_edge_graph(&g1,"306","723",1,0);
 add_edge_graph(&g1,"306","728",1,0);
 add_edge_graph(&g1,"306","733",1,0);
 add_edge_graph(&g1,"306","551",1,0);
 add_edge_graph(&g1,"306","753",1,0);
 add_edge_graph(&g1,"306","763",1,0);
 add_edge_graph(&g1,"306","766",1,0);
 add_edge_graph(&g1,"306","896",1,0);
 add_edge_graph(&g1,"306","774",1,0);
 add_edge_graph(&g1,"306","795",1,0);
 add_edge_graph(&g1,"306","798",1,0);
 add_edge_graph(&g1,"306","317",1,0);
 add_edge_graph(&g1,"306","324",1,0);
 add_edge_graph(&g1,"306","839",1,0);
 add_edge_graph(&g1,"306","328",1,0);
 add_edge_graph(&g1,"306","856",1,0);
 add_edge_graph(&g1,"306","859",1,0);
 add_edge_graph(&g1,"306","864",1,0);
 add_edge_graph(&g1,"306","371",1,0);
 add_edge_graph(&g1,"306","887",1,0);
 add_edge_graph(&g1,"306","379",1,0);
 add_edge_graph(&g1,"306","381",1,0);
 add_edge_graph(&g1,"306","894",1,0);
 add_edge_graph(&g1,"306","384",1,0);
 add_edge_graph(&g1,"306","900",1,0);
 add_edge_graph(&g1,"306","389",1,0);
 add_edge_graph(&g1,"306","395",1,0);
 add_edge_graph(&g1,"306","405",1,0);
 add_edge_graph(&g1,"306","921",1,0);
 add_edge_graph(&g1,"306","410",1,0);
 add_edge_graph(&g1,"306","928",1,0);
 add_edge_graph(&g1,"306","931",1,0);
 add_edge_graph(&g1,"306","420",1,0);
 add_edge_graph(&g1,"306","327",1,0);
 add_edge_graph(&g1,"306","940",1,0);
 add_edge_graph(&g1,"306","941",1,0);
 add_edge_graph(&g1,"306","430",1,0);
 add_edge_graph(&g1,"306","943",1,0);
 add_edge_graph(&g1,"306","434",1,0);
 add_edge_graph(&g1,"306","435",1,0);
 add_edge_graph(&g1,"306","437",1,0);
 add_edge_graph(&g1,"306","951",1,0);
 add_edge_graph(&g1,"306","441",1,0);
 add_edge_graph(&g1,"306","447",1,0);
 add_edge_graph(&g1,"306","449",1,0);
 add_edge_graph(&g1,"306","457",1,0);
 add_edge_graph(&g1,"306","970",1,0);
 add_edge_graph(&g1,"306","972",1,0);
 add_edge_graph(&g1,"306","462",1,0);
 add_edge_graph(&g1,"306","977",1,0);
 add_edge_graph(&g1,"306","466",1,0);
 add_edge_graph(&g1,"306","979",1,0);
 add_edge_graph(&g1,"306","471",1,0);
 add_edge_graph(&g1,"306","472",1,0);
 add_edge_graph(&g1,"306","480",1,0);
 add_edge_graph(&g1,"306","997",1,0);
 add_edge_graph(&g1,"306","998",1,0);
 add_edge_graph(&g1,"306","490",1,0);
 add_edge_graph(&g1,"306","511",1,0);
 add_edge_graph(&g1,"307","952",1,0);
 add_edge_graph(&g1,"307","997",1,0);
 add_edge_graph(&g1,"307","405",1,0);
 add_edge_graph(&g1,"307","837",1,0);
 add_edge_graph(&g1,"309","522",1,0);
 add_edge_graph(&g1,"309","639",1,0);
 add_edge_graph(&g1,"311","466",1,0);
 add_edge_graph(&g1,"311","420",1,0);
 add_edge_graph(&g1,"313","588",1,0);
 add_edge_graph(&g1,"314","721",1,0);
 add_edge_graph(&g1,"315","462",1,0);
 add_edge_graph(&g1,"315","471",1,0);
 add_edge_graph(&g1,"316","889",1,0);
 add_edge_graph(&g1,"316","837",1,0);
 add_edge_graph(&g1,"317","466",1,0);
 add_edge_graph(&g1,"317","471",1,0);
 add_edge_graph(&g1,"318","730",1,0);
 add_edge_graph(&g1,"318","837",1,0);
 add_edge_graph(&g1,"318","534",1,0);
 add_edge_graph(&g1,"319","709",1,0);
 add_edge_graph(&g1,"320","970",1,0);
 add_edge_graph(&g1,"323","471",1,0);
 add_edge_graph(&g1,"323","887",1,0);
 add_edge_graph(&g1,"324","449",1,0);
 add_edge_graph(&g1,"324","740",1,0);
 add_edge_graph(&g1,"324","391",1,0);
 add_edge_graph(&g1,"324","971",1,0);
 add_edge_graph(&g1,"324","690",1,0);
 add_edge_graph(&g1,"324","436",1,0);
 add_edge_graph(&g1,"324","590",1,0);
 add_edge_graph(&g1,"324","534",1,0);
 add_edge_graph(&g1,"324","795",1,0);
 add_edge_graph(&g1,"325","682",1,0);
 add_edge_graph(&g1,"325","405",1,0);
 add_edge_graph(&g1,"326","466",1,0);
 add_edge_graph(&g1,"326","471",1,0);
 add_edge_graph(&g1,"327","417",1,0);
 add_edge_graph(&g1,"327","392",1,0);
 add_edge_graph(&g1,"327","371",1,0);
 add_edge_graph(&g1,"327","468",1,0);
 add_edge_graph(&g1,"327","471",1,0);
 add_edge_graph(&g1,"327","763",1,0);
 add_edge_graph(&g1,"327","946",1,0);
 add_edge_graph(&g1,"328","968",1,0);
 add_edge_graph(&g1,"328","405",1,0);
 add_edge_graph(&g1,"328","371",1,0);
 add_edge_graph(&g1,"328","468",1,0);
 add_edge_graph(&g1,"328","437",1,0);
 add_edge_graph(&g1,"328","534",1,0);
 add_edge_graph(&g1,"328","985",1,0);
 add_edge_graph(&g1,"328","506",1,0);
 add_edge_graph(&g1,"328","447",1,0);
 add_edge_graph(&g1,"329","446",1,0);
 add_edge_graph(&g1,"329","471",1,0);
 add_edge_graph(&g1,"330","521",1,0);
 add_edge_graph(&g1,"331","964",1,0);
 add_edge_graph(&g1,"331","582",1,0);
 add_edge_graph(&g1,"331","780",1,0);
 add_edge_graph(&g1,"331","462",1,0);
 add_edge_graph(&g1,"331","371",1,0);
 add_edge_graph(&g1,"331","437",1,0);
 add_edge_graph(&g1,"331","534",1,0);
 add_edge_graph(&g1,"331","420",1,0);
 add_edge_graph(&g1,"331","922",1,0);
 add_edge_graph(&g1,"331","682",1,0);
 add_edge_graph(&g1,"332","521",1,0);
 add_edge_graph(&g1,"332","466",1,0);
 add_edge_graph(&g1,"332","597",1,0);
 add_edge_graph(&g1,"332","534",1,0);
 add_edge_graph(&g1,"332","859",1,0);
 add_edge_graph(&g1,"332","605",1,0);
 add_edge_graph(&g1,"333","466",1,0);
 add_edge_graph(&g1,"333","403",1,0);
 add_edge_graph(&g1,"334","405",1,0);
 add_edge_graph(&g1,"334","534",1,0);
 add_edge_graph(&g1,"334","535",1,0);
 add_edge_graph(&g1,"334","431",1,0);
 add_edge_graph(&g1,"334","807",1,0);
 add_edge_graph(&g1,"334","437",1,0);
 add_edge_graph(&g1,"334","574",1,0);
 add_edge_graph(&g1,"334","582",1,0);
 add_edge_graph(&g1,"334","970",1,0);
 add_edge_graph(&g1,"334","721",1,0);
 add_edge_graph(&g1,"334","979",1,0);
 add_edge_graph(&g1,"334","471",1,0);
 add_edge_graph(&g1,"334","730",1,0);
 add_edge_graph(&g1,"334","989",1,0);
 add_edge_graph(&g1,"334","866",1,0);
 add_edge_graph(&g1,"334","491",1,0);
 add_edge_graph(&g1,"334","371",1,0);
 add_edge_graph(&g1,"334","376",1,0);
 add_edge_graph(&g1,"334","682",1,0);
 add_edge_graph(&g1,"334","895",1,0);
 add_edge_graph(&g1,"337","979",1,0);
 add_edge_graph(&g1,"337","471",1,0);
 add_edge_graph(&g1,"338","466",1,0);
 add_edge_graph(&g1,"338","605",1,0);
 add_edge_graph(&g1,"338","437",1,0);
 add_edge_graph(&g1,"339","364",1,0);
 add_edge_graph(&g1,"340","940",1,0);
 add_edge_graph(&g1,"341","371",1,0);
 add_edge_graph(&g1,"342","471",1,0);
 add_edge_graph(&g1,"343","449",1,0);
 add_edge_graph(&g1,"343","437",1,0);
 add_edge_graph(&g1,"344","368",1,0);
 add_edge_graph(&g1,"344","371",1,0);
 add_edge_graph(&g1,"344","889",1,0);
 add_edge_graph(&g1,"344","927",1,0);
 add_edge_graph(&g1,"345","534",1,0);
 add_edge_graph(&g1,"345","471",1,0);
 add_edge_graph(&g1,"346","505",1,0);
 add_edge_graph(&g1,"346","922",1,0);
 add_edge_graph(&g1,"347","534",1,0);
 add_edge_graph(&g1,"348","471",1,0);
 add_edge_graph(&g1,"349","582",1,0);
 add_edge_graph(&g1,"349","455",1,0);
 add_edge_graph(&g1,"349","418",1,0);
 add_edge_graph(&g1,"349","371",1,0);
 add_edge_graph(&g1,"349","534",1,0);
 add_edge_graph(&g1,"349","471",1,0);
 add_edge_graph(&g1,"350","371",1,0);
 add_edge_graph(&g1,"351","992",1,0);
 add_edge_graph(&g1,"351","582",1,0);
 add_edge_graph(&g1,"351","983",1,0);
 add_edge_graph(&g1,"351","371",1,0);
 add_edge_graph(&g1,"351","884",1,0);
 add_edge_graph(&g1,"351","534",1,0);
 add_edge_graph(&g1,"351","951",1,0);
 add_edge_graph(&g1,"351","420",1,0);
 add_edge_graph(&g1,"351","794",1,0);
 add_edge_graph(&g1,"351","471",1,0);
 add_edge_graph(&g1,"353","479",1,0);
 add_edge_graph(&g1,"355","770",1,0);
 add_edge_graph(&g1,"355","965",1,0);
 add_edge_graph(&g1,"356","634",1,0);
 add_edge_graph(&g1,"356","605",1,0);
 add_edge_graph(&g1,"357","582",1,0);
 add_edge_graph(&g1,"357","591",1,0);
 add_edge_graph(&g1,"357","371",1,0);
 add_edge_graph(&g1,"357","534",1,0);
 add_edge_graph(&g1,"357","471",1,0);
 add_edge_graph(&g1,"357","506",1,0);
 add_edge_graph(&g1,"358","371",1,0);
 add_edge_graph(&g1,"359","371",1,0);
 add_edge_graph(&g1,"359","471",1,0);
 add_edge_graph(&g1,"360","792",1,0);
 add_edge_graph(&g1,"361","904",1,0);
 add_edge_graph(&g1,"361","871",1,0);
 add_edge_graph(&g1,"362","534",1,0);
 add_edge_graph(&g1,"363","471",1,0);
 add_edge_graph(&g1,"364","800",1,0);
 add_edge_graph(&g1,"364","912",1,0);
 add_edge_graph(&g1,"364","466",1,0);
 add_edge_graph(&g1,"364","405",1,0);
 add_edge_graph(&g1,"364","471",1,0);
 add_edge_graph(&g1,"364","765",1,0);
 add_edge_graph(&g1,"364","574",1,0);
 add_edge_graph(&g1,"365","449",1,0);
 add_edge_graph(&g1,"365","716",1,0);
 add_edge_graph(&g1,"366","574",1,0);
 add_edge_graph(&g1,"367","585",1,0);
 add_edge_graph(&g1,"367","471",1,0);
 add_edge_graph(&g1,"368","471",1,0);
 add_edge_graph(&g1,"369","371",1,0);
 add_edge_graph(&g1,"370","785",1,0);
 add_edge_graph(&g1,"370","437",1,0);
 add_edge_graph(&g1,"371","512",1,0);
 add_edge_graph(&g1,"371","521",1,0);
 add_edge_graph(&g1,"371","522",1,0);
 add_edge_graph(&g1,"371","733",1,0);
 add_edge_graph(&g1,"371","524",1,0);
 add_edge_graph(&g1,"371","514",1,0);
 add_edge_graph(&g1,"371","988",1,0);
 add_edge_graph(&g1,"371","533",1,0);
 add_edge_graph(&g1,"371","537",1,0);
 add_edge_graph(&g1,"371","517",1,0);
 add_edge_graph(&g1,"371","545",1,0);
 add_edge_graph(&g1,"371","550",1,0);
 add_edge_graph(&g1,"371","552",1,0);
 add_edge_graph(&g1,"371","553",1,0);
 add_edge_graph(&g1,"371","558",1,0);
 add_edge_graph(&g1,"371","605",1,0);
 add_edge_graph(&g1,"371","561",1,0);
 add_edge_graph(&g1,"371","562",1,0);
 add_edge_graph(&g1,"371","563",1,0);
 add_edge_graph(&g1,"371","565",1,0);
 add_edge_graph(&g1,"371","839",1,0);
 add_edge_graph(&g1,"371","572",1,0);
 add_edge_graph(&g1,"371","574",1,0);
 add_edge_graph(&g1,"371","581",1,0);
 add_edge_graph(&g1,"371","582",1,0);
 add_edge_graph(&g1,"371","583",1,0);
 add_edge_graph(&g1,"371","780",1,0);
 add_edge_graph(&g1,"371","923",1,0);
 add_edge_graph(&g1,"371","590",1,0);
 add_edge_graph(&g1,"371","525",1,0);
 add_edge_graph(&g1,"371","597",1,0);
 add_edge_graph(&g1,"371","697",1,0);
 add_edge_graph(&g1,"371","600",1,0);
 add_edge_graph(&g1,"371","761",1,0);
 add_edge_graph(&g1,"371","613",1,0);
 add_edge_graph(&g1,"371","612",1,0);
 add_edge_graph(&g1,"371","795",1,0);
 add_edge_graph(&g1,"371","871",1,0);
 add_edge_graph(&g1,"371","630",1,0);
 add_edge_graph(&g1,"371","633",1,0);
 add_edge_graph(&g1,"371","789",1,0);
 add_edge_graph(&g1,"371","644",1,0);
 add_edge_graph(&g1,"371","647",1,0);
 add_edge_graph(&g1,"371","649",1,0);
 add_edge_graph(&g1,"371","659",1,0);
 add_edge_graph(&g1,"371","793",1,0);
 add_edge_graph(&g1,"371","668",1,0);
 add_edge_graph(&g1,"371","534",1,0);
 add_edge_graph(&g1,"371","626",1,0);
 add_edge_graph(&g1,"371","686",1,0);
 add_edge_graph(&g1,"371","687",1,0);
 add_edge_graph(&g1,"371","690",1,0);
 add_edge_graph(&g1,"371","415",1,0);
 add_edge_graph(&g1,"371","543",1,0);
 add_edge_graph(&g1,"371","701",1,0);
 add_edge_graph(&g1,"371","704",1,0);
 add_edge_graph(&g1,"371","708",1,0);
 add_edge_graph(&g1,"371","711",1,0);
 add_edge_graph(&g1,"371","715",1,0);
 add_edge_graph(&g1,"371","717",1,0);
 add_edge_graph(&g1,"371","723",1,0);
 add_edge_graph(&g1,"371","719",1,0);
 add_edge_graph(&g1,"371","979",1,0);
 add_edge_graph(&g1,"371","746",1,0);
 add_edge_graph(&g1,"371","749",1,0);
 add_edge_graph(&g1,"371","750",1,0);
 add_edge_graph(&g1,"371","637",1,0);
 add_edge_graph(&g1,"371","752",1,0);
 add_edge_graph(&g1,"371","753",1,0);
 add_edge_graph(&g1,"371","755",1,0);
 add_edge_graph(&g1,"371","763",1,0);
 add_edge_graph(&g1,"371","764",1,0);
 add_edge_graph(&g1,"371","768",1,0);
 add_edge_graph(&g1,"371","861",1,0);
 add_edge_graph(&g1,"371","784",1,0);
 add_edge_graph(&g1,"371","787",1,0);
 add_edge_graph(&g1,"371","791",1,0);
 add_edge_graph(&g1,"371","799",1,0);
 add_edge_graph(&g1,"371","802",1,0);
 add_edge_graph(&g1,"371","862",1,0);
 add_edge_graph(&g1,"371","794",1,0);
 add_edge_graph(&g1,"371","982",1,0);
 add_edge_graph(&g1,"371","735",1,0);
 add_edge_graph(&g1,"371","906",1,0);
 add_edge_graph(&g1,"371","830",1,0);
 add_edge_graph(&g1,"371","480",1,0);
 add_edge_graph(&g1,"371","566",1,0);
 add_edge_graph(&g1,"371","842",1,0);
 add_edge_graph(&g1,"371","844",1,0);
 add_edge_graph(&g1,"371","845",1,0);
 add_edge_graph(&g1,"371","848",1,0);
 add_edge_graph(&g1,"371","995",1,0);
 add_edge_graph(&g1,"371","855",1,0);
 add_edge_graph(&g1,"371","608",1,0);
 add_edge_graph(&g1,"371","858",1,0);
 add_edge_graph(&g1,"371","873",1,0);
 add_edge_graph(&g1,"371","403",1,0);
 add_edge_graph(&g1,"371","888",1,0);
 add_edge_graph(&g1,"371","377",1,0);
 add_edge_graph(&g1,"371","379",1,0);
 add_edge_graph(&g1,"371","381",1,0);
 add_edge_graph(&g1,"371","383",1,0);
 add_edge_graph(&g1,"371","898",1,0);
 add_edge_graph(&g1,"371","747",1,0);
 add_edge_graph(&g1,"371","900",1,0);
 add_edge_graph(&g1,"371","902",1,0);
 add_edge_graph(&g1,"371","904",1,0);
 add_edge_graph(&g1,"371","394",1,0);
 add_edge_graph(&g1,"371","907",1,0);
 add_edge_graph(&g1,"371","397",1,0);
 add_edge_graph(&g1,"371","911",1,0);
 add_edge_graph(&g1,"371","401",1,0);
 add_edge_graph(&g1,"371","915",1,0);
 add_edge_graph(&g1,"371","404",1,0);
 add_edge_graph(&g1,"371","405",1,0);
 add_edge_graph(&g1,"371","919",1,0);
 add_edge_graph(&g1,"371","921",1,0);
 add_edge_graph(&g1,"371","922",1,0);
 add_edge_graph(&g1,"371","751",1,0);
 add_edge_graph(&g1,"371","410",1,0);
 add_edge_graph(&g1,"371","926",1,0);
 add_edge_graph(&g1,"371","837",1,0);
 add_edge_graph(&g1,"371","931",1,0);
 add_edge_graph(&g1,"371","420",1,0);
 add_edge_graph(&g1,"371","426",1,0);
 add_edge_graph(&g1,"371","939",1,0);
 add_edge_graph(&g1,"371","428",1,0);
 add_edge_graph(&g1,"371","430",1,0);
 add_edge_graph(&g1,"371","645",1,0);
 add_edge_graph(&g1,"371","948",1,0);
 add_edge_graph(&g1,"371","437",1,0);
 add_edge_graph(&g1,"371","619",1,0);
 add_edge_graph(&g1,"371","696",1,0);
 add_edge_graph(&g1,"371","671",1,0);
 add_edge_graph(&g1,"371","446",1,0);
 add_edge_graph(&g1,"371","959",1,0);
 add_edge_graph(&g1,"371","449",1,0);
 add_edge_graph(&g1,"371","962",1,0);
 add_edge_graph(&g1,"371","451",1,0);
 add_edge_graph(&g1,"371","966",1,0);
 add_edge_graph(&g1,"371","968",1,0);
 add_edge_graph(&g1,"371","969",1,0);
 add_edge_graph(&g1,"371","970",1,0);
 add_edge_graph(&g1,"371","971",1,0);
 add_edge_graph(&g1,"371","972",1,0);
 add_edge_graph(&g1,"371","461",1,0);
 add_edge_graph(&g1,"371","976",1,0);
 add_edge_graph(&g1,"371","977",1,0);
 add_edge_graph(&g1,"371","466",1,0);
 add_edge_graph(&g1,"371","467",1,0);
 add_edge_graph(&g1,"371","470",1,0);
 add_edge_graph(&g1,"371","471",1,0);
 add_edge_graph(&g1,"371","472",1,0);
 add_edge_graph(&g1,"371","985",1,0);
 add_edge_graph(&g1,"371","986",1,0);
 add_edge_graph(&g1,"371","591",1,0);
 add_edge_graph(&g1,"371","476",1,0);
 add_edge_graph(&g1,"371","989",1,0);
 add_edge_graph(&g1,"371","990",1,0);
 add_edge_graph(&g1,"371","991",1,0);
 add_edge_graph(&g1,"371","992",1,0);
 add_edge_graph(&g1,"371","483",1,0);
 add_edge_graph(&g1,"371","996",1,0);
 add_edge_graph(&g1,"371","997",1,0);
 add_edge_graph(&g1,"371","998",1,0);
 add_edge_graph(&g1,"371","593",1,0);
 add_edge_graph(&g1,"371","456",1,0);
 add_edge_graph(&g1,"371","492",1,0);
 add_edge_graph(&g1,"371","594",1,0);
 add_edge_graph(&g1,"371","495",1,0);
 add_edge_graph(&g1,"371","496",1,0);
 add_edge_graph(&g1,"371","497",1,0);
 add_edge_graph(&g1,"371","498",1,0);
 add_edge_graph(&g1,"371","500",1,0);
 add_edge_graph(&g1,"371","501",1,0);
 add_edge_graph(&g1,"371","504",1,0);
 add_edge_graph(&g1,"371","506",1,0);
 add_edge_graph(&g1,"371","682",1,0);
 add_edge_graph(&g1,"371","511",1,0);
 add_edge_graph(&g1,"372","574",1,0);
 add_edge_graph(&g1,"373","477",1,0);
 add_edge_graph(&g1,"374","837",1,0);
 add_edge_graph(&g1,"374","714",1,0);
 add_edge_graph(&g1,"374","428",1,0);
 add_edge_graph(&g1,"374","626",1,0);
 add_edge_graph(&g1,"374","526",1,0);
 add_edge_graph(&g1,"374","466",1,0);
 add_edge_graph(&g1,"374","659",1,0);
 add_edge_graph(&g1,"374","571",1,0);
 add_edge_graph(&g1,"375","534",1,0);
 add_edge_graph(&g1,"376","682",1,0);
 add_edge_graph(&g1,"379","582",1,0);
 add_edge_graph(&g1,"379","742",1,0);
 add_edge_graph(&g1,"379","649",1,0);
 add_edge_graph(&g1,"379","682",1,0);
 add_edge_graph(&g1,"379","637",1,0);
 add_edge_graph(&g1,"381","928",1,0);
 add_edge_graph(&g1,"381","418",1,0);
 add_edge_graph(&g1,"381","597",1,0);
 add_edge_graph(&g1,"381","682",1,0);
 add_edge_graph(&g1,"381","910",1,0);
 add_edge_graph(&g1,"381","593",1,0);
 add_edge_graph(&g1,"381","574",1,0);
 add_edge_graph(&g1,"381","534",1,0);
 add_edge_graph(&g1,"381","471",1,0);
 add_edge_graph(&g1,"381","729",1,0);
 add_edge_graph(&g1,"381","571",1,0);
 add_edge_graph(&g1,"381","702",1,0);
 add_edge_graph(&g1,"383","619",1,0);
 add_edge_graph(&g1,"383","534",1,0);
 add_edge_graph(&g1,"385","971",1,0);
 add_edge_graph(&g1,"385","558",1,0);
 add_edge_graph(&g1,"387","935",1,0);
 add_edge_graph(&g1,"389","997",1,0);
 add_edge_graph(&g1,"389","553",1,0);
 add_edge_graph(&g1,"389","682",1,0);
 add_edge_graph(&g1,"389","815",1,0);
 add_edge_graph(&g1,"389","403",1,0);
 add_edge_graph(&g1,"389","420",1,0);
 add_edge_graph(&g1,"389","574",1,0);
 add_edge_graph(&g1,"390","939",1,0);
 add_edge_graph(&g1,"390","534",1,0);
 add_edge_graph(&g1,"391","582",1,0);
 add_edge_graph(&g1,"392","471",1,0);
 add_edge_graph(&g1,"392","437",1,0);
 add_edge_graph(&g1,"393","545",1,0);
 add_edge_graph(&g1,"393","787",1,0);
 add_edge_graph(&g1,"393","462",1,0);
 add_edge_graph(&g1,"394","534",1,0);
 add_edge_graph(&g1,"395","806",1,0);
 add_edge_graph(&g1,"395","428",1,0);
 add_edge_graph(&g1,"395","692",1,0);
 add_edge_graph(&g1,"395","437",1,0);
 add_edge_graph(&g1,"395","471",1,0);
 add_edge_graph(&g1,"395","605",1,0);
 add_edge_graph(&g1,"395","405",1,0);
 add_edge_graph(&g1,"396","433",1,0);
 add_edge_graph(&g1,"396","582",1,0);
 add_edge_graph(&g1,"396","471",1,0);
 add_edge_graph(&g1,"397","737",1,0);
 add_edge_graph(&g1,"398","434",1,0);
 add_edge_graph(&g1,"399","572",1,0);
 add_edge_graph(&g1,"400","466",1,0);
 add_edge_graph(&g1,"401","441",1,0);
 add_edge_graph(&g1,"402","971",1,0);
 add_edge_graph(&g1,"402","534",1,0);
 add_edge_graph(&g1,"403","709",1,0);
 add_edge_graph(&g1,"403","878",1,0);
 add_edge_graph(&g1,"404","434",1,0);
 add_edge_graph(&g1,"405","768",1,0);
 add_edge_graph(&g1,"405","619",1,0);
 add_edge_graph(&g1,"405","797",1,0);
 add_edge_graph(&g1,"405","642",1,0);
 add_edge_graph(&g1,"405","910",1,0);
 add_edge_graph(&g1,"405","528",1,0);
 add_edge_graph(&g1,"405","534",1,0);
 add_edge_graph(&g1,"405","410",1,0);
 add_edge_graph(&g1,"405","420",1,0);
 add_edge_graph(&g1,"405","683",1,0);
 add_edge_graph(&g1,"405","428",1,0);
 add_edge_graph(&g1,"405","558",1,0);
 add_edge_graph(&g1,"405","816",1,0);
 add_edge_graph(&g1,"405","436",1,0);
 add_edge_graph(&g1,"405","437",1,0);
 add_edge_graph(&g1,"405","820",1,0);
 add_edge_graph(&g1,"405","965",1,0);
 add_edge_graph(&g1,"405","574",1,0);
 add_edge_graph(&g1,"405","704",1,0);
 add_edge_graph(&g1,"405","452",1,0);
 add_edge_graph(&g1,"405","582",1,0);
 add_edge_graph(&g1,"405","585",1,0);
 add_edge_graph(&g1,"405","525",1,0);
 add_edge_graph(&g1,"405","592",1,0);
 add_edge_graph(&g1,"405","721",1,0);
 add_edge_graph(&g1,"405","466",1,0);
 add_edge_graph(&g1,"405","723",1,0);
 add_edge_graph(&g1,"405","462",1,0);
 add_edge_graph(&g1,"405","563",1,0);
 add_edge_graph(&g1,"405","471",1,0);
 add_edge_graph(&g1,"405","719",1,0);
 add_edge_graph(&g1,"405","610",1,0);
 add_edge_graph(&g1,"405","741",1,0);
 add_edge_graph(&g1,"405","486",1,0);
 add_edge_graph(&g1,"405","593",1,0);
 add_edge_graph(&g1,"405","997",1,0);
 add_edge_graph(&g1,"405","722",1,0);
 add_edge_graph(&g1,"405","981",1,0);
 add_edge_graph(&g1,"405","515",1,0);
 add_edge_graph(&g1,"405","504",1,0);
 add_edge_graph(&g1,"405","763",1,0);
 add_edge_graph(&g1,"405","682",1,0);
 add_edge_graph(&g1,"405","597",1,0);
 add_edge_graph(&g1,"406","480",1,0);
 add_edge_graph(&g1,"406","704",1,0);
 add_edge_graph(&g1,"406","566",1,0);
 add_edge_graph(&g1,"406","681",1,0);
 add_edge_graph(&g1,"406","682",1,0);
 add_edge_graph(&g1,"406","844",1,0);
 add_edge_graph(&g1,"406","466",1,0);
 add_edge_graph(&g1,"406","534",1,0);
 add_edge_graph(&g1,"406","471",1,0);
 add_edge_graph(&g1,"407","923",1,0);
 add_edge_graph(&g1,"408","582",1,0);
 add_edge_graph(&g1,"408","534",1,0);
 add_edge_graph(&g1,"409","660",1,0);
 add_edge_graph(&g1,"409","540",1,0);
 add_edge_graph(&g1,"410","627",1,0);
 add_edge_graph(&g1,"410","709",1,0);
 add_edge_graph(&g1,"410","582",1,0);
 add_edge_graph(&g1,"410","998",1,0);
 add_edge_graph(&g1,"410","471",1,0);
 add_edge_graph(&g1,"410","899",1,0);
 add_edge_graph(&g1,"410","574",1,0);
 add_edge_graph(&g1,"411","449",1,0);
 add_edge_graph(&g1,"411","669",1,0);
 add_edge_graph(&g1,"413","878",1,0);
 add_edge_graph(&g1,"414","582",1,0);
 add_edge_graph(&g1,"415","480",1,0);
 add_edge_graph(&g1,"415","567",1,0);
 add_edge_graph(&g1,"416","854",1,0);
 add_edge_graph(&g1,"417","928",1,0);
 add_edge_graph(&g1,"417","704",1,0);
 add_edge_graph(&g1,"417","582",1,0);
 add_edge_graph(&g1,"417","428",1,0);
 add_edge_graph(&g1,"417","723",1,0);
 add_edge_graph(&g1,"417","534",1,0);
 add_edge_graph(&g1,"417","471",1,0);
 add_edge_graph(&g1,"417","926",1,0);
 add_edge_graph(&g1,"418","768",1,0);
 add_edge_graph(&g1,"418","619",1,0);
 add_edge_graph(&g1,"418","944",1,0);
 add_edge_graph(&g1,"418","692",1,0);
 add_edge_graph(&g1,"418","437",1,0);
 add_edge_graph(&g1,"418","471",1,0);
 add_edge_graph(&g1,"420","904",1,0);
 add_edge_graph(&g1,"420","919",1,0);
 add_edge_graph(&g1,"420","941",1,0);
 add_edge_graph(&g1,"420","532",1,0);
 add_edge_graph(&g1,"420","534",1,0);
 add_edge_graph(&g1,"420","925",1,0);
 add_edge_graph(&g1,"420","773",1,0);
 add_edge_graph(&g1,"420","928",1,0);
 add_edge_graph(&g1,"420","545",1,0);
 add_edge_graph(&g1,"420","808",1,0);
 add_edge_graph(&g1,"420","427",1,0);
 add_edge_graph(&g1,"420","820",1,0);
 add_edge_graph(&g1,"420","437",1,0);
 add_edge_graph(&g1,"420","959",1,0);
 add_edge_graph(&g1,"420","608",1,0);
 add_edge_graph(&g1,"420","837",1,0);
 add_edge_graph(&g1,"420","966",1,0);
 add_edge_graph(&g1,"420","462",1,0);
 add_edge_graph(&g1,"420","466",1,0);
 add_edge_graph(&g1,"420","724",1,0);
 add_edge_graph(&g1,"420","910",1,0);
 add_edge_graph(&g1,"420","855",1,0);
 add_edge_graph(&g1,"420","761",1,0);
 add_edge_graph(&g1,"420","471",1,0);
 add_edge_graph(&g1,"420","989",1,0);
 add_edge_graph(&g1,"420","480",1,0);
 add_edge_graph(&g1,"420","994",1,0);
 add_edge_graph(&g1,"420","485",1,0);
 add_edge_graph(&g1,"420","492",1,0);
 add_edge_graph(&g1,"420","574",1,0);
 add_edge_graph(&g1,"420","754",1,0);
 add_edge_graph(&g1,"420","997",1,0);
 add_edge_graph(&g1,"421","534",1,0);
 add_edge_graph(&g1,"421","447",1,0);
 add_edge_graph(&g1,"422","471",1,0);
 add_edge_graph(&g1,"423","744",1,0);
 add_edge_graph(&g1,"423","682",1,0);
 add_edge_graph(&g1,"424","712",1,0);
 add_edge_graph(&g1,"426","997",1,0);
 add_edge_graph(&g1,"426","524",1,0);
 add_edge_graph(&g1,"426","436",1,0);
 add_edge_graph(&g1,"426","446",1,0);
 add_edge_graph(&g1,"426","574",1,0);
 add_edge_graph(&g1,"428","768",1,0);
 add_edge_graph(&g1,"428","897",1,0);
 add_edge_graph(&g1,"428","534",1,0);
 add_edge_graph(&g1,"428","928",1,0);
 add_edge_graph(&g1,"428","558",1,0);
 add_edge_graph(&g1,"428","437",1,0);
 add_edge_graph(&g1,"428","582",1,0);
 add_edge_graph(&g1,"428","972",1,0);
 add_edge_graph(&g1,"428","471",1,0);
 add_edge_graph(&g1,"428","480",1,0);
 add_edge_graph(&g1,"428","744",1,0);
 add_edge_graph(&g1,"428","633",1,0);
 add_edge_graph(&g1,"430","471",1,0);
 add_edge_graph(&g1,"431","704",1,0);
 add_edge_graph(&g1,"431","545",1,0);
 add_edge_graph(&g1,"432","723",1,0);
 add_edge_graph(&g1,"433","496",1,0);
 add_edge_graph(&g1,"434","534",1,0);
 add_edge_graph(&g1,"434","605",1,0);
 add_edge_graph(&g1,"435","928",1,0);
 add_edge_graph(&g1,"435","582",1,0);
 add_edge_graph(&g1,"435","744",1,0);
 add_edge_graph(&g1,"435","682",1,0);
 add_edge_graph(&g1,"435","534",1,0);
 add_edge_graph(&g1,"435","855",1,0);
 add_edge_graph(&g1,"435","540",1,0);
 add_edge_graph(&g1,"435","605",1,0);
 add_edge_graph(&g1,"436","682",1,0);
 add_edge_graph(&g1,"437","514",1,0);
 add_edge_graph(&g1,"437","534",1,0);
 add_edge_graph(&g1,"437","543",1,0);
 add_edge_graph(&g1,"437","558",1,0);
 add_edge_graph(&g1,"437","569",1,0);
 add_edge_graph(&g1,"437","571",1,0);
 add_edge_graph(&g1,"437","574",1,0);
 add_edge_graph(&g1,"437","581",1,0);
 add_edge_graph(&g1,"437","582",1,0);
 add_edge_graph(&g1,"437","593",1,0);
 add_edge_graph(&g1,"437","595",1,0);
 add_edge_graph(&g1,"437","605",1,0);
 add_edge_graph(&g1,"437","612",1,0);
 add_edge_graph(&g1,"437","613",1,0);
 add_edge_graph(&g1,"437","636",1,0);
 add_edge_graph(&g1,"437","637",1,0);
 add_edge_graph(&g1,"437","645",1,0);
 add_edge_graph(&g1,"437","654",1,0);
 add_edge_graph(&g1,"437","656",1,0);
 add_edge_graph(&g1,"437","682",1,0);
 add_edge_graph(&g1,"437","692",1,0);
 add_edge_graph(&g1,"437","703",1,0);
 add_edge_graph(&g1,"437","723",1,0);
 add_edge_graph(&g1,"437","744",1,0);
 add_edge_graph(&g1,"437","978",1,0);
 add_edge_graph(&g1,"437","751",1,0);
 add_edge_graph(&g1,"437","766",1,0);
 add_edge_graph(&g1,"437","768",1,0);
 add_edge_graph(&g1,"437","772",1,0);
 add_edge_graph(&g1,"437","780",1,0);
 add_edge_graph(&g1,"437","794",1,0);
 add_edge_graph(&g1,"437","818",1,0);
 add_edge_graph(&g1,"437","814",1,0);
 add_edge_graph(&g1,"437","837",1,0);
 add_edge_graph(&g1,"437","850",1,0);
 add_edge_graph(&g1,"437","871",1,0);
 add_edge_graph(&g1,"437","882",1,0);
 add_edge_graph(&g1,"437","883",1,0);
 add_edge_graph(&g1,"437","888",1,0);
 add_edge_graph(&g1,"437","917",1,0);
 add_edge_graph(&g1,"437","911",1,0);
 add_edge_graph(&g1,"437","918",1,0);
 add_edge_graph(&g1,"437","928",1,0);
 add_edge_graph(&g1,"437","933",1,0);
 add_edge_graph(&g1,"437","939",1,0);
 add_edge_graph(&g1,"437","945",1,0);
 add_edge_graph(&g1,"437","443",1,0);
 add_edge_graph(&g1,"437","449",1,0);
 add_edge_graph(&g1,"437","964",1,0);
 add_edge_graph(&g1,"437","457",1,0);
 add_edge_graph(&g1,"437","972",1,0);
 add_edge_graph(&g1,"437","466",1,0);
 add_edge_graph(&g1,"437","979",1,0);
 add_edge_graph(&g1,"437","471",1,0);
 add_edge_graph(&g1,"437","472",1,0);
 add_edge_graph(&g1,"437","932",1,0);
 add_edge_graph(&g1,"437","989",1,0);
 add_edge_graph(&g1,"437","482",1,0);
 add_edge_graph(&g1,"437","998",1,0);
 add_edge_graph(&g1,"437","510",1,0);
 add_edge_graph(&g1,"438","534",1,0);
 add_edge_graph(&g1,"440","605",1,0);
 add_edge_graph(&g1,"442","466",1,0);
 add_edge_graph(&g1,"442","979",1,0);
 add_edge_graph(&g1,"443","832",1,0);
 add_edge_graph(&g1,"444","471",1,0);
 add_edge_graph(&g1,"444","667",1,0);
 add_edge_graph(&g1,"446","682",1,0);
 add_edge_graph(&g1,"446","779",1,0);
 add_edge_graph(&g1,"446","543",1,0);
 add_edge_graph(&g1,"447","457",1,0);
 add_edge_graph(&g1,"447","534",1,0);
 add_edge_graph(&g1,"448","650",1,0);
 add_edge_graph(&g1,"449","524",1,0);
 add_edge_graph(&g1,"449","534",1,0);
 add_edge_graph(&g1,"449","807",1,0);
 add_edge_graph(&g1,"449","940",1,0);
 add_edge_graph(&g1,"449","698",1,0);
 add_edge_graph(&g1,"449","705",1,0);
 add_edge_graph(&g1,"449","453",1,0);
 add_edge_graph(&g1,"449","972",1,0);
 add_edge_graph(&g1,"449","723",1,0);
 add_edge_graph(&g1,"449","690",1,0);
 add_edge_graph(&g1,"449","471",1,0);
 add_edge_graph(&g1,"449","608",1,0);
 add_edge_graph(&g1,"449","988",1,0);
 add_edge_graph(&g1,"449","480",1,0);
 add_edge_graph(&g1,"449","998",1,0);
 add_edge_graph(&g1,"449","502",1,0);
 add_edge_graph(&g1,"449","950",1,0);
 add_edge_graph(&g1,"450","471",1,0);
 add_edge_graph(&g1,"452","582",1,0);
 add_edge_graph(&g1,"452","682",1,0);
 add_edge_graph(&g1,"452","633",1,0);
 add_edge_graph(&g1,"457","872",1,0);
 add_edge_graph(&g1,"457","979",1,0);
 add_edge_graph(&g1,"458","471",1,0);
 add_edge_graph(&g1,"459","471",1,0);
 add_edge_graph(&g1,"460","466",1,0);
 add_edge_graph(&g1,"462","928",1,0);
 add_edge_graph(&g1,"462","534",1,0);
 add_edge_graph(&g1,"462","471",1,0);
 add_edge_graph(&g1,"462","472",1,0);
 add_edge_graph(&g1,"462","475",1,0);
 add_edge_graph(&g1,"463","574",1,0);
 add_edge_graph(&g1,"465","534",1,0);
 add_edge_graph(&g1,"465","558",1,0);
 add_edge_graph(&g1,"465","582",1,0);
 add_edge_graph(&g1,"466","534",1,0);
 add_edge_graph(&g1,"466","522",1,0);
 add_edge_graph(&g1,"466","855",1,0);
 add_edge_graph(&g1,"466","780",1,0);
 add_edge_graph(&g1,"466","723",1,0);
 add_edge_graph(&g1,"466","643",1,0);
 add_edge_graph(&g1,"466","665",1,0);
 add_edge_graph(&g1,"466","543",1,0);
 add_edge_graph(&g1,"466","794",1,0);
 add_edge_graph(&g1,"466","545",1,0);
 add_edge_graph(&g1,"466","667",1,0);
 add_edge_graph(&g1,"466","679",1,0);
 add_edge_graph(&g1,"466","582",1,0);
 add_edge_graph(&g1,"466","690",1,0);
 add_edge_graph(&g1,"466","675",1,0);
 add_edge_graph(&g1,"466","928",1,0);
 add_edge_graph(&g1,"466","706",1,0);
 add_edge_graph(&g1,"466","843",1,0);
 add_edge_graph(&g1,"466","837",1,0);
 add_edge_graph(&g1,"466","597",1,0);
 add_edge_graph(&g1,"466","592",1,0);
 add_edge_graph(&g1,"466","471",1,0);
 add_edge_graph(&g1,"466","729",1,0);
 add_edge_graph(&g1,"466","979",1,0);
 add_edge_graph(&g1,"466","991",1,0);
 add_edge_graph(&g1,"466","571",1,0);
 add_edge_graph(&g1,"466","868",1,0);
 add_edge_graph(&g1,"466","997",1,0);
 add_edge_graph(&g1,"466","870",1,0);
 add_edge_graph(&g1,"466","487",1,0);
 add_edge_graph(&g1,"466","528",1,0);
 add_edge_graph(&g1,"466","829",1,0);
 add_edge_graph(&g1,"466","496",1,0);
 add_edge_graph(&g1,"466","677",1,0);
 add_edge_graph(&g1,"466","885",1,0);
 add_edge_graph(&g1,"466","605",1,0);
 add_edge_graph(&g1,"466","891",1,0);
 add_edge_graph(&g1,"466","682",1,0);
 add_edge_graph(&g1,"467","480",1,0);
 add_edge_graph(&g1,"467","771",1,0);
 add_edge_graph(&g1,"467","534",1,0);
 add_edge_graph(&g1,"467","816",1,0);
 add_edge_graph(&g1,"467","723",1,0);
 add_edge_graph(&g1,"468","514",1,0);
 add_edge_graph(&g1,"468","709",1,0);
 add_edge_graph(&g1,"468","984",1,0);
 add_edge_graph(&g1,"468","665",1,0);
 add_edge_graph(&g1,"469","793",1,0);
 add_edge_graph(&g1,"470","928",1,0);
 add_edge_graph(&g1,"470","574",1,0);
 add_edge_graph(&g1,"470","471",1,0);
 add_edge_graph(&g1,"471","513",1,0);
 add_edge_graph(&g1,"471","521",1,0);
 add_edge_graph(&g1,"471","522",1,0);
 add_edge_graph(&g1,"471","525",1,0);
 add_edge_graph(&g1,"471","534",1,0);
 add_edge_graph(&g1,"471","536",1,0);
 add_edge_graph(&g1,"471","539",1,0);
 add_edge_graph(&g1,"471","543",1,0);
 add_edge_graph(&g1,"471","545",1,0);
 add_edge_graph(&g1,"471","558",1,0);
 add_edge_graph(&g1,"471","563",1,0);
 add_edge_graph(&g1,"471","564",1,0);
 add_edge_graph(&g1,"471","565",1,0);
 add_edge_graph(&g1,"471","571",1,0);
 add_edge_graph(&g1,"471","573",1,0);
 add_edge_graph(&g1,"471","574",1,0);
 add_edge_graph(&g1,"471","579",1,0);
 add_edge_graph(&g1,"471","582",1,0);
 add_edge_graph(&g1,"471","597",1,0);
 add_edge_graph(&g1,"471","590",1,0);
 add_edge_graph(&g1,"471","591",1,0);
 add_edge_graph(&g1,"471","593",1,0);
 add_edge_graph(&g1,"471","599",1,0);
 add_edge_graph(&g1,"471","605",1,0);
 add_edge_graph(&g1,"471","613",1,0);
 add_edge_graph(&g1,"471","608",1,0);
 add_edge_graph(&g1,"471","619",1,0);
 add_edge_graph(&g1,"471","624",1,0);
 add_edge_graph(&g1,"471","625",1,0);
 add_edge_graph(&g1,"471","787",1,0);
 add_edge_graph(&g1,"471","636",1,0);
 add_edge_graph(&g1,"471","961",1,0);
 add_edge_graph(&g1,"471","876",1,0);
 add_edge_graph(&g1,"471","651",1,0);
 add_edge_graph(&g1,"471","653",1,0);
 add_edge_graph(&g1,"471","659",1,0);
 add_edge_graph(&g1,"471","667",1,0);
 add_edge_graph(&g1,"471","704",1,0);
 add_edge_graph(&g1,"471","807",1,0);
 add_edge_graph(&g1,"471","696",1,0);
 add_edge_graph(&g1,"471","705",1,0);
 add_edge_graph(&g1,"471","971",1,0);
 add_edge_graph(&g1,"471","719",1,0);
 add_edge_graph(&g1,"471","720",1,0);
 add_edge_graph(&g1,"471","723",1,0);
 add_edge_graph(&g1,"471","733",1,0);
 add_edge_graph(&g1,"471","748",1,0);
 add_edge_graph(&g1,"471","783",1,0);
 add_edge_graph(&g1,"471","795",1,0);
 add_edge_graph(&g1,"471","820",1,0);
 add_edge_graph(&g1,"471","828",1,0);
 add_edge_graph(&g1,"471","863",1,0);
 add_edge_graph(&g1,"471","837",1,0);
 add_edge_graph(&g1,"471","855",1,0);
 add_edge_graph(&g1,"471","856",1,0);
 add_edge_graph(&g1,"471","867",1,0);
 add_edge_graph(&g1,"471","691",1,0);
 add_edge_graph(&g1,"471","888",1,0);
 add_edge_graph(&g1,"471","890",1,0);
 add_edge_graph(&g1,"471","895",1,0);
 add_edge_graph(&g1,"471","918",1,0);
 add_edge_graph(&g1,"471","911",1,0);
 add_edge_graph(&g1,"471","934",1,0);
 add_edge_graph(&g1,"471","921",1,0);
 add_edge_graph(&g1,"471","883",1,0);
 add_edge_graph(&g1,"471","922",1,0);
 add_edge_graph(&g1,"471","928",1,0);
 add_edge_graph(&g1,"471","941",1,0);
 add_edge_graph(&g1,"471","950",1,0);
 add_edge_graph(&g1,"471","952",1,0);
 add_edge_graph(&g1,"471","957",1,0);
 add_edge_graph(&g1,"471","966",1,0);
 add_edge_graph(&g1,"471","968",1,0);
 add_edge_graph(&g1,"471","972",1,0);
 add_edge_graph(&g1,"471","979",1,0);
 add_edge_graph(&g1,"471","987",1,0);
 add_edge_graph(&g1,"471","476",1,0);
 add_edge_graph(&g1,"471","478",1,0);
 add_edge_graph(&g1,"471","997",1,0);
 add_edge_graph(&g1,"471","492",1,0);
 add_edge_graph(&g1,"471","496",1,0);
 add_edge_graph(&g1,"471","497",1,0);
 add_edge_graph(&g1,"471","501",1,0);
 add_edge_graph(&g1,"471","502",1,0);
 add_edge_graph(&g1,"471","506",1,0);
 add_edge_graph(&g1,"471","682",1,0);
 add_edge_graph(&g1,"471","511",1,0);
 add_edge_graph(&g1,"472","512",1,0);
 add_edge_graph(&g1,"472","678",1,0);
 add_edge_graph(&g1,"472","534",1,0);
 add_edge_graph(&g1,"473","666",1,0);
 add_edge_graph(&g1,"476","873",1,0);
 add_edge_graph(&g1,"478","574",1,0);
 add_edge_graph(&g1,"480","770",1,0);
 add_edge_graph(&g1,"480","534",1,0);
 add_edge_graph(&g1,"480","582",1,0);
 add_edge_graph(&g1,"480","502",1,0);
 add_edge_graph(&g1,"480","572",1,0);
 add_edge_graph(&g1,"480","574",1,0);
 add_edge_graph(&g1,"481","534",1,0);
 add_edge_graph(&g1,"484","501",1,0);
 add_edge_graph(&g1,"486","760",1,0);
 add_edge_graph(&g1,"487","966",1,0);
 add_edge_graph(&g1,"488","534",1,0);
 add_edge_graph(&g1,"490","545",1,0);
 add_edge_graph(&g1,"490","582",1,0);
 add_edge_graph(&g1,"490","929",1,0);
 add_edge_graph(&g1,"490","726",1,0);
 add_edge_graph(&g1,"492","928",1,0);
 add_edge_graph(&g1,"492","773",1,0);
 add_edge_graph(&g1,"492","536",1,0);
 add_edge_graph(&g1,"492","632",1,0);
 add_edge_graph(&g1,"492","970",1,0);
 add_edge_graph(&g1,"492","686",1,0);
 add_edge_graph(&g1,"492","723",1,0);
 add_edge_graph(&g1,"492","497",1,0);
 add_edge_graph(&g1,"492","878",1,0);
 add_edge_graph(&g1,"492","534",1,0);
 add_edge_graph(&g1,"492","504",1,0);
 add_edge_graph(&g1,"492","605",1,0);
 add_edge_graph(&g1,"493","723",1,0);
 add_edge_graph(&g1,"494","980",1,0);
 add_edge_graph(&g1,"495","655",1,0);
 add_edge_graph(&g1,"496","600",1,0);
 add_edge_graph(&g1,"496","682",1,0);
 add_edge_graph(&g1,"496","825",1,0);
 add_edge_graph(&g1,"496","604",1,0);
 add_edge_graph(&g1,"497","739",1,0);
 add_edge_graph(&g1,"497","744",1,0);
 add_edge_graph(&g1,"497","854",1,0);
 add_edge_graph(&g1,"498","972",1,0);
 add_edge_graph(&g1,"498","582",1,0);
 add_edge_graph(&g1,"500","647",1,0);
 add_edge_graph(&g1,"501","711",1,0);
 add_edge_graph(&g1,"503","972",1,0);
 add_edge_graph(&g1,"504","768",1,0);
 add_edge_graph(&g1,"504","682",1,0);
 add_edge_graph(&g1,"504","690",1,0);
 add_edge_graph(&g1,"504","534",1,0);
 add_edge_graph(&g1,"504","574",1,0);
 add_edge_graph(&g1,"504","938",1,0);
 add_edge_graph(&g1,"506","620",1,0);
 add_edge_graph(&g1,"506","972",1,0);
 add_edge_graph(&g1,"506","558",1,0);
 add_edge_graph(&g1,"506","921",1,0);
 add_edge_graph(&g1,"506","635",1,0);
 add_edge_graph(&g1,"506","574",1,0);
 add_edge_graph(&g1,"508","554",1,0);
 add_edge_graph(&g1,"509","667",1,0);
 add_edge_graph(&g1,"510","723",1,0);
 add_edge_graph(&g1,"510","605",1,0);
 add_edge_graph(&g1,"510","959",1,0);
 add_edge_graph(&g1,"511","928",1,0);
 add_edge_graph(&g1,"511","737",1,0);
 add_edge_graph(&g1,"511","534",1,0);
 add_edge_graph(&g1,"511","591",1,0);
 add_edge_graph(&g1,"512","522",1,0);
 add_edge_graph(&g1,"514","534",1,0);
 add_edge_graph(&g1,"515","582",1,0);
 add_edge_graph(&g1,"515","682",1,0);
 add_edge_graph(&g1,"515","876",1,0);
 add_edge_graph(&g1,"515","534",1,0);
 add_edge_graph(&g1,"515","571",1,0);
 add_edge_graph(&g1,"515","574",1,0);
 add_edge_graph(&g1,"516","720",1,0);
 add_edge_graph(&g1,"518","556",1,0);
 add_edge_graph(&g1,"518","997",1,0);
 add_edge_graph(&g1,"519","534",1,0);
 add_edge_graph(&g1,"520","534",1,0);
 add_edge_graph(&g1,"521","608",1,0);
 add_edge_graph(&g1,"521","737",1,0);
 add_edge_graph(&g1,"521","522",1,0);
 add_edge_graph(&g1,"521","586",1,0);
 add_edge_graph(&g1,"521","582",1,0);
 add_edge_graph(&g1,"521","682",1,0);
 add_edge_graph(&g1,"521","887",1,0);
 add_edge_graph(&g1,"521","978",1,0);
 add_edge_graph(&g1,"521","820",1,0);
 add_edge_graph(&g1,"521","534",1,0);
 add_edge_graph(&g1,"521","799",1,0);
 add_edge_graph(&g1,"521","991",1,0);
 add_edge_graph(&g1,"522","768",1,0);
 add_edge_graph(&g1,"522","656",1,0);
 add_edge_graph(&g1,"522","786",1,0);
 add_edge_graph(&g1,"522","534",1,0);
 add_edge_graph(&g1,"522","541",1,0);
 add_edge_graph(&g1,"522","928",1,0);
 add_edge_graph(&g1,"522","582",1,0);
 add_edge_graph(&g1,"522","711",1,0);
 add_edge_graph(&g1,"522","844",1,0);
 add_edge_graph(&g1,"522","723",1,0);
 add_edge_graph(&g1,"522","744",1,0);
 add_edge_graph(&g1,"522","619",1,0);
 add_edge_graph(&g1,"522","630",1,0);
 add_edge_graph(&g1,"524","628",1,0);
 add_edge_graph(&g1,"524","564",1,0);
 add_edge_graph(&g1,"525","704",1,0);
 add_edge_graph(&g1,"525","545",1,0);
 add_edge_graph(&g1,"525","582",1,0);
 add_edge_graph(&g1,"525","743",1,0);
 add_edge_graph(&g1,"525","776",1,0);
 add_edge_graph(&g1,"525","608",1,0);
 add_edge_graph(&g1,"527","723",1,0);
 add_edge_graph(&g1,"528","672",1,0);
 add_edge_graph(&g1,"528","928",1,0);
 add_edge_graph(&g1,"528","676",1,0);
 add_edge_graph(&g1,"528","534",1,0);
 add_edge_graph(&g1,"528","574",1,0);
 add_edge_graph(&g1,"529","608",1,0);
 add_edge_graph(&g1,"530","682",1,0);
 add_edge_graph(&g1,"531","998",1,0);
 add_edge_graph(&g1,"532","545",1,0);
 add_edge_graph(&g1,"532","732",1,0);
 add_edge_graph(&g1,"534","558",1,0);
 add_edge_graph(&g1,"534","949",1,0);
 add_edge_graph(&g1,"534","576",1,0);
 add_edge_graph(&g1,"534","582",1,0);
 add_edge_graph(&g1,"534","583",1,0);
 add_edge_graph(&g1,"534","585",1,0);
 add_edge_graph(&g1,"534","588",1,0);
 add_edge_graph(&g1,"534","593",1,0);
 add_edge_graph(&g1,"534","596",1,0);
 add_edge_graph(&g1,"534","605",1,0);
 add_edge_graph(&g1,"534","608",1,0);
 add_edge_graph(&g1,"534","612",1,0);
 add_edge_graph(&g1,"534","618",1,0);
 add_edge_graph(&g1,"534","619",1,0);
 add_edge_graph(&g1,"534","648",1,0);
 add_edge_graph(&g1,"534","793",1,0);
 add_edge_graph(&g1,"534","664",1,0);
 add_edge_graph(&g1,"534","672",1,0);
 add_edge_graph(&g1,"534","675",1,0);
 add_edge_graph(&g1,"534","710",1,0);
 add_edge_graph(&g1,"534","682",1,0);
 add_edge_graph(&g1,"534","689",1,0);
 add_edge_graph(&g1,"534","701",1,0);
 add_edge_graph(&g1,"534","704",1,0);
 add_edge_graph(&g1,"534","705",1,0);
 add_edge_graph(&g1,"534","709",1,0);
 add_edge_graph(&g1,"534","711",1,0);
 add_edge_graph(&g1,"534","715",1,0);
 add_edge_graph(&g1,"534","716",1,0);
 add_edge_graph(&g1,"534","717",1,0);
 add_edge_graph(&g1,"534","721",1,0);
 add_edge_graph(&g1,"534","722",1,0);
 add_edge_graph(&g1,"534","723",1,0);
 add_edge_graph(&g1,"534","733",1,0);
 add_edge_graph(&g1,"534","744",1,0);
 add_edge_graph(&g1,"534","745",1,0);
 add_edge_graph(&g1,"534","979",1,0);
 add_edge_graph(&g1,"534","759",1,0);
 add_edge_graph(&g1,"534","761",1,0);
 add_edge_graph(&g1,"534","775",1,0);
 add_edge_graph(&g1,"534","779",1,0);
 add_edge_graph(&g1,"534","795",1,0);
 add_edge_graph(&g1,"534","799",1,0);
 add_edge_graph(&g1,"534","800",1,0);
 add_edge_graph(&g1,"534","807",1,0);
 add_edge_graph(&g1,"534","812",1,0);
 add_edge_graph(&g1,"534","822",1,0);
 add_edge_graph(&g1,"534","829",1,0);
 add_edge_graph(&g1,"534","837",1,0);
 add_edge_graph(&g1,"534","858",1,0);
 add_edge_graph(&g1,"534","597",1,0);
 add_edge_graph(&g1,"534","876",1,0);
 add_edge_graph(&g1,"534","877",1,0);
 add_edge_graph(&g1,"534","881",1,0);
 add_edge_graph(&g1,"534","574",1,0);
 add_edge_graph(&g1,"534","892",1,0);
 add_edge_graph(&g1,"534","922",1,0);
 add_edge_graph(&g1,"534","926",1,0);
 add_edge_graph(&g1,"534","928",1,0);
 add_edge_graph(&g1,"534","934",1,0);
 add_edge_graph(&g1,"534","942",1,0);
 add_edge_graph(&g1,"534","929",1,0);
 add_edge_graph(&g1,"534","844",1,0);
 add_edge_graph(&g1,"534","970",1,0);
 add_edge_graph(&g1,"534","973",1,0);
 add_edge_graph(&g1,"534","975",1,0);
 add_edge_graph(&g1,"534","994",1,0);
 add_edge_graph(&g1,"534","997",1,0);
 add_edge_graph(&g1,"534","630",1,0);
 add_edge_graph(&g1,"534","545",1,0);
 add_edge_graph(&g1,"535","682",1,0);
 add_edge_graph(&g1,"536","834",1,0);
 add_edge_graph(&g1,"540","800",1,0);
 add_edge_graph(&g1,"541","692",1,0);
 add_edge_graph(&g1,"542","899",1,0);
 add_edge_graph(&g1,"543","729",1,0);
 add_edge_graph(&g1,"543","605",1,0);
 add_edge_graph(&g1,"545","855",1,0);
 add_edge_graph(&g1,"545","657",1,0);
 add_edge_graph(&g1,"545","935",1,0);
 add_edge_graph(&g1,"545","936",1,0);
 add_edge_graph(&g1,"545","682",1,0);
 add_edge_graph(&g1,"545","574",1,0);
 add_edge_graph(&g1,"545","582",1,0);
 add_edge_graph(&g1,"545","713",1,0);
 add_edge_graph(&g1,"545","844",1,0);
 add_edge_graph(&g1,"545","723",1,0);
 add_edge_graph(&g1,"545","785",1,0);
 add_edge_graph(&g1,"545","759",1,0);
 add_edge_graph(&g1,"548","998",1,0);
 add_edge_graph(&g1,"549","943",1,0);
 add_edge_graph(&g1,"552","980",1,0);
 add_edge_graph(&g1,"553","637",1,0);
 add_edge_graph(&g1,"554","785",1,0);
 add_edge_graph(&g1,"555","828",1,0);
 add_edge_graph(&g1,"556","686",1,0);
 add_edge_graph(&g1,"557","780",1,0);
 add_edge_graph(&g1,"557","607",1,0);
 add_edge_graph(&g1,"558","665",1,0);
 add_edge_graph(&g1,"558","682",1,0);
 add_edge_graph(&g1,"558","690",1,0);
 add_edge_graph(&g1,"558","956",1,0);
 add_edge_graph(&g1,"558","582",1,0);
 add_edge_graph(&g1,"558","722",1,0);
 add_edge_graph(&g1,"558","979",1,0);
 add_edge_graph(&g1,"558","859",1,0);
 add_edge_graph(&g1,"558","604",1,0);
 add_edge_graph(&g1,"558","605",1,0);
 add_edge_graph(&g1,"558","997",1,0);
 add_edge_graph(&g1,"562","972",1,0);
 add_edge_graph(&g1,"563","768",1,0);
 add_edge_graph(&g1,"563","682",1,0);
 add_edge_graph(&g1,"563","876",1,0);
 add_edge_graph(&g1,"563","605",1,0);
 add_edge_graph(&g1,"564","744",1,0);
 add_edge_graph(&g1,"565","968",1,0);
 add_edge_graph(&g1,"568","811",1,0);
 add_edge_graph(&g1,"568","843",1,0);
 add_edge_graph(&g1,"569","605",1,0);
 add_edge_graph(&g1,"570","928",1,0);
 add_edge_graph(&g1,"571","682",1,0);
 add_edge_graph(&g1,"571","741",1,0);
 add_edge_graph(&g1,"572","677",1,0);
 add_edge_graph(&g1,"573","631",1,0);
 add_edge_graph(&g1,"574","576",1,0);
 add_edge_graph(&g1,"574","682",1,0);
 add_edge_graph(&g1,"574","928",1,0);
 add_edge_graph(&g1,"574","675",1,0);
 add_edge_graph(&g1,"574","717",1,0);
 add_edge_graph(&g1,"574","692",1,0);
 add_edge_graph(&g1,"574","696",1,0);
 add_edge_graph(&g1,"574","787",1,0);
 add_edge_graph(&g1,"574","704",1,0);
 add_edge_graph(&g1,"574","643",1,0);
 add_edge_graph(&g1,"574","582",1,0);
 add_edge_graph(&g1,"574","844",1,0);
 add_edge_graph(&g1,"574","856",1,0);
 add_edge_graph(&g1,"574","591",1,0);
 add_edge_graph(&g1,"574","988",1,0);
 add_edge_graph(&g1,"574","605",1,0);
 add_edge_graph(&g1,"574","997",1,0);
 add_edge_graph(&g1,"574","993",1,0);
 add_edge_graph(&g1,"574","613",1,0);
 add_edge_graph(&g1,"574","975",1,0);
 add_edge_graph(&g1,"574","746",1,0);
 add_edge_graph(&g1,"574","807",1,0);
 add_edge_graph(&g1,"574","633",1,0);
 add_edge_graph(&g1,"574","639",1,0);
 add_edge_graph(&g1,"577","833",1,0);
 add_edge_graph(&g1,"578","809",1,0);
 add_edge_graph(&g1,"582","949",1,0);
 add_edge_graph(&g1,"582","585",1,0);
 add_edge_graph(&g1,"582","591",1,0);
 add_edge_graph(&g1,"582","593",1,0);
 add_edge_graph(&g1,"582","605",1,0);
 add_edge_graph(&g1,"582","652",1,0);
 add_edge_graph(&g1,"582","663",1,0);
 add_edge_graph(&g1,"582","678",1,0);
 add_edge_graph(&g1,"582","682",1,0);
 add_edge_graph(&g1,"582","704",1,0);
 add_edge_graph(&g1,"582","712",1,0);
 add_edge_graph(&g1,"582","722",1,0);
 add_edge_graph(&g1,"582","723",1,0);
 add_edge_graph(&g1,"582","729",1,0);
 add_edge_graph(&g1,"582","780",1,0);
 add_edge_graph(&g1,"582","785",1,0);
 add_edge_graph(&g1,"582","813",1,0);
 add_edge_graph(&g1,"582","817",1,0);
 add_edge_graph(&g1,"582","821",1,0);
 add_edge_graph(&g1,"582","822",1,0);
 add_edge_graph(&g1,"582","826",1,0);
 add_edge_graph(&g1,"582","828",1,0);
 add_edge_graph(&g1,"582","926",1,0);
 add_edge_graph(&g1,"582","841",1,0);
 add_edge_graph(&g1,"582","859",1,0);
 add_edge_graph(&g1,"582","860",1,0);
 add_edge_graph(&g1,"582","884",1,0);
 add_edge_graph(&g1,"582","893",1,0);
 add_edge_graph(&g1,"582","917",1,0);
 add_edge_graph(&g1,"582","900",1,0);
 add_edge_graph(&g1,"582","905",1,0);
 add_edge_graph(&g1,"582","914",1,0);
 add_edge_graph(&g1,"582","915",1,0);
 add_edge_graph(&g1,"582","928",1,0);
 add_edge_graph(&g1,"582","970",1,0);
 add_edge_graph(&g1,"582","972",1,0);
 add_edge_graph(&g1,"584","952",1,0);
 add_edge_graph(&g1,"585","597",1,0);
 add_edge_graph(&g1,"591","682",1,0);
 add_edge_graph(&g1,"593","928",1,0);
 add_edge_graph(&g1,"593","615",1,0);
 add_edge_graph(&g1,"593","686",1,0);
 add_edge_graph(&g1,"593","837",1,0);
 add_edge_graph(&g1,"593","625",1,0);
 add_edge_graph(&g1,"594","928",1,0);
 add_edge_graph(&g1,"594","605",1,0);
 add_edge_graph(&g1,"597","736",1,0);
 add_edge_graph(&g1,"597","612",1,0);
 add_edge_graph(&g1,"597","904",1,0);
 add_edge_graph(&g1,"597","785",1,0);
 add_edge_graph(&g1,"597","948",1,0);
 add_edge_graph(&g1,"597","972",1,0);
 add_edge_graph(&g1,"598","878",1,0);
 add_edge_graph(&g1,"599","926",1,0);
 add_edge_graph(&g1,"603","692",1,0);
 add_edge_graph(&g1,"604","705",1,0);
 add_edge_graph(&g1,"605","726",1,0);
 add_edge_graph(&g1,"605","682",1,0);
 add_edge_graph(&g1,"605","793",1,0);
 add_edge_graph(&g1,"605","749",1,0);
 add_edge_graph(&g1,"605","788",1,0);
 add_edge_graph(&g1,"605","729",1,0);
 add_edge_graph(&g1,"605","606",1,0);
 add_edge_graph(&g1,"605","928",1,0);
 add_edge_graph(&g1,"605","940",1,0);
 add_edge_graph(&g1,"605","822",1,0);
 add_edge_graph(&g1,"605","837",1,0);
 add_edge_graph(&g1,"605","847",1,0);
 add_edge_graph(&g1,"605","952",1,0);
 add_edge_graph(&g1,"605","910",1,0);
 add_edge_graph(&g1,"605","608",1,0);
 add_edge_graph(&g1,"605","997",1,0);
 add_edge_graph(&g1,"605","873",1,0);
 add_edge_graph(&g1,"605","761",1,0);
 add_edge_graph(&g1,"605","762",1,0);
 add_edge_graph(&g1,"605","765",1,0);
 add_edge_graph(&g1,"608","704",1,0);
 add_edge_graph(&g1,"608","753",1,0);
 add_edge_graph(&g1,"608","989",1,0);
 add_edge_graph(&g1,"608","692",1,0);
 add_edge_graph(&g1,"610","798",1,0);
 add_edge_graph(&g1,"612","928",1,0);
 add_edge_graph(&g1,"616","928",1,0);
 add_edge_graph(&g1,"616","936",1,0);
 add_edge_graph(&g1,"616","966",1,0);
 add_edge_graph(&g1,"617","971",1,0);
 add_edge_graph(&g1,"619","651",1,0);
 add_edge_graph(&g1,"619","653",1,0);
 add_edge_graph(&g1,"619","786",1,0);
 add_edge_graph(&g1,"620","876",1,0);
 add_edge_graph(&g1,"620","954",1,0);
 add_edge_graph(&g1,"622","920",1,0);
 add_edge_graph(&g1,"624","844",1,0);
 add_edge_graph(&g1,"624","770",1,0);
 add_edge_graph(&g1,"629","682",1,0);
 add_edge_graph(&g1,"631","704",1,0);
 add_edge_graph(&g1,"633","894",1,0);
 add_edge_graph(&g1,"637","928",1,0);
 add_edge_graph(&g1,"638","753",1,0);
 add_edge_graph(&g1,"639","852",1,0);
 add_edge_graph(&g1,"640","715",1,0);
 add_edge_graph(&g1,"642","768",1,0);
 add_edge_graph(&g1,"642","993",1,0);
 add_edge_graph(&g1,"644","953",1,0);
 add_edge_graph(&g1,"646","704",1,0);
 add_edge_graph(&g1,"648","768",1,0);
 add_edge_graph(&g1,"651","979",1,0);
 add_edge_graph(&g1,"652","928",1,0);
 add_edge_graph(&g1,"653","940",1,0);
 add_edge_graph(&g1,"658","682",1,0);
 add_edge_graph(&g1,"659","682",1,0);
 add_edge_graph(&g1,"660","928",1,0);
 add_edge_graph(&g1,"661","895",1,0);
 add_edge_graph(&g1,"663","886",1,0);
 add_edge_graph(&g1,"665","696",1,0);
 add_edge_graph(&g1,"665","838",1,0);
 add_edge_graph(&g1,"666","957",1,0);
 add_edge_graph(&g1,"670","682",1,0);
 add_edge_graph(&g1,"674","935",1,0);
 add_edge_graph(&g1,"675","807",1,0);
 add_edge_graph(&g1,"681","690",1,0);
 add_edge_graph(&g1,"682","768",1,0);
 add_edge_graph(&g1,"682","837",1,0);
 add_edge_graph(&g1,"682","926",1,0);
 add_edge_graph(&g1,"682","936",1,0);
 add_edge_graph(&g1,"682","711",1,0);
 add_edge_graph(&g1,"682","956",1,0);
 add_edge_graph(&g1,"682","704",1,0);
 add_edge_graph(&g1,"682","971",1,0);
 add_edge_graph(&g1,"682","854",1,0);
 add_edge_graph(&g1,"682","787",1,0);
 add_edge_graph(&g1,"682","770",1,0);
 add_edge_graph(&g1,"682","915",1,0);
 add_edge_graph(&g1,"682","734",1,0);
 add_edge_graph(&g1,"682","865",1,0);
 add_edge_graph(&g1,"682","997",1,0);
 add_edge_graph(&g1,"682","998",1,0);
 add_edge_graph(&g1,"682","743",1,0);
 add_edge_graph(&g1,"682","723",1,0);
 add_edge_graph(&g1,"682","753",1,0);
 add_edge_graph(&g1,"682","686",1,0);
 add_edge_graph(&g1,"684","686",1,0);
 add_edge_graph(&g1,"685","801",1,0);
 add_edge_graph(&g1,"685","692",1,0);
 add_edge_graph(&g1,"686","998",1,0);
 add_edge_graph(&g1,"686","999",1,0);
 add_edge_graph(&g1,"687","872",1,0);
 add_edge_graph(&g1,"687","859",1,0);
 add_edge_graph(&g1,"690","782",1,0);
 add_edge_graph(&g1,"691","994",1,0);
 add_edge_graph(&g1,"691","887",1,0);
 add_edge_graph(&g1,"692","843",1,0);
 add_edge_graph(&g1,"692","878",1,0);
 add_edge_graph(&g1,"692","696",1,0);
 add_edge_graph(&g1,"693","963",1,0);
 add_edge_graph(&g1,"693","700",1,0);
 add_edge_graph(&g1,"695","879",1,0);
 add_edge_graph(&g1,"696","926",1,0);
 add_edge_graph(&g1,"701","966",1,0);
 add_edge_graph(&g1,"704","909",1,0);
 add_edge_graph(&g1,"704","928",1,0);
 add_edge_graph(&g1,"704","824",1,0);
 add_edge_graph(&g1,"704","864",1,0);
 add_edge_graph(&g1,"705","900",1,0);
 add_edge_graph(&g1,"705","871",1,0);
 add_edge_graph(&g1,"705","743",1,0);
 add_edge_graph(&g1,"705","958",1,0);
 add_edge_graph(&g1,"707","723",1,0);
 add_edge_graph(&g1,"709","997",1,0);
 add_edge_graph(&g1,"709","723",1,0);
 add_edge_graph(&g1,"709","796",1,0);
 add_edge_graph(&g1,"709","990",1,0);
 add_edge_graph(&g1,"711","839",1,0);
 add_edge_graph(&g1,"717","933",1,0);
 add_edge_graph(&g1,"720","867",1,0);
 add_edge_graph(&g1,"721","878",1,0);
 add_edge_graph(&g1,"721","989",1,0);
 add_edge_graph(&g1,"722","966",1,0);
 add_edge_graph(&g1,"723","823",1,0);
 add_edge_graph(&g1,"723","922",1,0);
 add_edge_graph(&g1,"723","837",1,0);
 add_edge_graph(&g1,"723","930",1,0);
 add_edge_graph(&g1,"723","839",1,0);
 add_edge_graph(&g1,"723","970",1,0);
 add_edge_graph(&g1,"723","862",1,0);
 add_edge_graph(&g1,"723","997",1,0);
 add_edge_graph(&g1,"723","741",1,0);
 add_edge_graph(&g1,"723","743",1,0);
 add_edge_graph(&g1,"725","979",1,0);
 add_edge_graph(&g1,"726","902",1,0);
 add_edge_graph(&g1,"726","790",1,0);
 add_edge_graph(&g1,"726","807",1,0);
 add_edge_graph(&g1,"729","770",1,0);
 add_edge_graph(&g1,"731","928",1,0);
 add_edge_graph(&g1,"733","750",1,0);
 add_edge_graph(&g1,"736","945",1,0);
 add_edge_graph(&g1,"738","928",1,0);
 add_edge_graph(&g1,"741","768",1,0);
 add_edge_graph(&g1,"741","998",1,0);
 add_edge_graph(&g1,"743","746",1,0);
 add_edge_graph(&g1,"744","768",1,0);
 add_edge_graph(&g1,"751","998",1,0);
 add_edge_graph(&g1,"753","842",1,0);
 add_edge_graph(&g1,"755","844",1,0);
 add_edge_graph(&g1,"757","786",1,0);
 add_edge_graph(&g1,"763","768",1,0);
 add_edge_graph(&g1,"763","844",1,0);
 add_edge_graph(&g1,"768","998",1,0);
 add_edge_graph(&g1,"768","997",1,0);
 add_edge_graph(&g1,"768","857",1,0);
 add_edge_graph(&g1,"769","926",1,0);
 add_edge_graph(&g1,"772","900",1,0);
 add_edge_graph(&g1,"778","913",1,0);
 add_edge_graph(&g1,"780","993",1,0);
 add_edge_graph(&g1,"780","873",1,0);
 add_edge_graph(&g1,"784","928",1,0);
 add_edge_graph(&g1,"787","908",1,0);
 add_edge_graph(&g1,"787","966",1,0);
 add_edge_graph(&g1,"787","904",1,0);
 add_edge_graph(&g1,"788","971",1,0);
 add_edge_graph(&g1,"789","997",1,0);
 add_edge_graph(&g1,"791","793",1,0);
 add_edge_graph(&g1,"793","914",1,0);
 add_edge_graph(&g1,"793","819",1,0);
 add_edge_graph(&g1,"797","887",1,0);
 add_edge_graph(&g1,"803","966",1,0);
 add_edge_graph(&g1,"804","971",1,0);
 add_edge_graph(&g1,"808","831",1,0);
 add_edge_graph(&g1,"811","904",1,0);
 add_edge_graph(&g1,"812","937",1,0);
 add_edge_graph(&g1,"822","967",1,0);
 add_edge_graph(&g1,"825","843",1,0);
 add_edge_graph(&g1,"827","970",1,0);
 add_edge_graph(&g1,"828","874",1,0);
 add_edge_graph(&g1,"829","847",1,0);
 add_edge_graph(&g1,"835","997",1,0);
 add_edge_graph(&g1,"837","966",1,0);
 add_edge_graph(&g1,"843","952",1,0);
 add_edge_graph(&g1,"843","888",1,0);
 add_edge_graph(&g1,"844","998",1,0);
 add_edge_graph(&g1,"844","903",1,0);
 add_edge_graph(&g1,"845","997",1,0);
 add_edge_graph(&g1,"850","928",1,0);
 add_edge_graph(&g1,"851","855",1,0);
 add_edge_graph(&g1,"855","942",1,0);
 add_edge_graph(&g1,"855","924",1,0);
 add_edge_graph(&g1,"857","905",1,0);
 add_edge_graph(&g1,"858","926",1,0);
 add_edge_graph(&g1,"870","926",1,0);
 add_edge_graph(&g1,"878","971",1,0);
 add_edge_graph(&g1,"880","997",1,0);
 add_edge_graph(&g1,"888","972",1,0);
 add_edge_graph(&g1,"902","997",1,0);
 add_edge_graph(&g1,"904","971",1,0);
 add_edge_graph(&g1,"904","950",1,0);
 add_edge_graph(&g1,"904","989",1,0);
 add_edge_graph(&g1,"908","979",1,0);
 add_edge_graph(&g1,"917","970",1,0);
 add_edge_graph(&g1,"926","974",1,0);
 add_edge_graph(&g1,"926","998",1,0);
 add_edge_graph(&g1,"928","966",1,0);
 add_edge_graph(&g1,"928","982",1,0);
 add_edge_graph(&g1,"932","937",1,0);
 add_edge_graph(&g1,"951","988",1,0);
 add_edge_graph(&g1,"955","970",1,0);
 add_edge_graph(&g1,"957","997",1,0);
 add_edge_graph(&g1,"961","977",1,0);
 add_edge_graph(&g1,"972","997",1,0);
 add_edge_graph(&g1,"977","998",1,0);
 add_edge_graph(&g1,"979","997",1,0);
 add_edge_graph(&g1,"997","998",1,0);
 add_edge_graph(&g1,"997","999",1,0);
 
 
 struct node_list *  nl;
 double * bh=betweeness_brandes(&g1,true,0);
 
 double * bh_c=betwenness_heuristic(&g1,false);
 //double * bh_c2=betwenness_heuristic(&g1,true);
 
 
 for(nl=g1.nodes.head;nl!=0;nl=nl->next){
 struct node_graph * ng=(struct node_graph*)nl->content;
 
 
 printf("%s:\t%f \t%f\t%d\t%1.50f\n",
 //printf("%s:\t%1.50f \t%1.50f\t%d\t%1.50f\n",
 ng->name,
 bh[ng->node_graph_id], 
 bh_c[ng->node_graph_id],
 bh[ng->node_graph_id]==bh_c[ng->node_graph_id],
 -bh[ng->node_graph_id]+bh_c[ng->node_graph_id]);
 if((-bh[ng->node_graph_id]+bh_c[ng->node_graph_id])!=0){
 printf("%1.50f\n",
 //printf("%s:\t%1.50f \t%1.50f\t%d\t%1.50f\n",
 -bh[ng->node_graph_id]+bh_c[ng->node_graph_id]);
 }
 }
 free(bh);
 free(bh_c);
 //free(bh_c2);
 free_graph(&g1);
 return 0;
 }*/

