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
bool multithread=false;
bool stop_computing_if_unchanged=false;

/**
 * Whether we are using heuristic in case of single connected component.
 * From test based on 100 cliques of 200 nodes, results are
 * heuristic: mean 0.21064373254776, var: 4.5616834192685475e-06
 * original: mean 0.21824131727218629, var 9.6816689202285048e-06
 * so @use_heu_on_single_biconnected is set to true,to improve performance
 */
bool use_heu_on_single_biconnected=true;

/*
 * Not used !!!
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
            //ret_val[ng->node_graph_id]=round_decimal(ret_val[ng->node_graph_id]);
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
        if(sg->connected_components.size>1||use_heu_on_single_biconnected){
            
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
            // ret_val[i]=round_decimal(ret_val[i]);
            
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
//TODO: test this, different results on Python. Problem on compiled version?
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
    add_edge_graph(&g1,"0","531",6.02122644228,0);
    add_edge_graph(&g1,"1","193",0.547260118989,0);
    add_edge_graph(&g1,"1","786",0.478635624018,0);
    add_edge_graph(&g1,"1","289",6.58483995225,0);
    add_edge_graph(&g1,"2","120",5.27620358804,0);
    add_edge_graph(&g1,"2","193",6.63153452062,0);
    add_edge_graph(&g1,"3","256",6.22689064286,0);
    add_edge_graph(&g1,"3","129",7.13465198008,0);
    add_edge_graph(&g1,"3","131",7.80612143528,0);
    add_edge_graph(&g1,"3","139",3.9425499117,0);
    add_edge_graph(&g1,"3","527",5.42830684092,0);
    add_edge_graph(&g1,"3","274",7.44963314258,0);
    add_edge_graph(&g1,"3","152",8.25503147273,0);
    add_edge_graph(&g1,"3","537",7.43526906902,0);
    add_edge_graph(&g1,"3","795",5.3312088965,0);
    add_edge_graph(&g1,"3","930",0.882388184327,0);
    add_edge_graph(&g1,"3","35",9.59872063185,0);
    add_edge_graph(&g1,"3","36",3.40427932051,0);
    add_edge_graph(&g1,"3","557",2.57098131879,0);
    add_edge_graph(&g1,"3","174",7.95878525219,0);
    add_edge_graph(&g1,"3","304",1.40510576603,0);
    add_edge_graph(&g1,"3","824",1.16027557494,0);
    add_edge_graph(&g1,"3","786",9.82239059406,0);
    add_edge_graph(&g1,"3","571",8.54036288718,0);
    add_edge_graph(&g1,"3","330",8.35650817728,0);
    add_edge_graph(&g1,"3","202",2.94841942565,0);
    add_edge_graph(&g1,"3","596",3.62572646807,0);
    add_edge_graph(&g1,"3","474",8.66345081598,0);
    add_edge_graph(&g1,"3","485",5.67659767515,0);
    add_edge_graph(&g1,"3","877",4.6113674435,0);
    add_edge_graph(&g1,"3","108",9.05390628088,0);
    add_edge_graph(&g1,"3","658",5.2280786218,0);
    add_edge_graph(&g1,"3","890",2.76264060225,0);
    add_edge_graph(&g1,"3","380",4.53320786759,0);
    add_edge_graph(&g1,"4","745",9.80378361,0);
    add_edge_graph(&g1,"4","474",8.3542970669,0);
    add_edge_graph(&g1,"4","139",0.964290688059,0);
    add_edge_graph(&g1,"4","733",4.08278662812,0);
    add_edge_graph(&g1,"4","473",9.73872387448,0);
    add_edge_graph(&g1,"5","952",9.17094959835,0);
    add_edge_graph(&g1,"5","81",1.90320513645,0);
    add_edge_graph(&g1,"6","36",3.36184838782,0);
    add_edge_graph(&g1,"6","828",1.56083866427,0);
    add_edge_graph(&g1,"7","230",5.54910099177,0);
    add_edge_graph(&g1,"7","390",2.53262399764,0);
    add_edge_graph(&g1,"7","874",2.61601600186,0);
    add_edge_graph(&g1,"7","139",6.32998537216,0);
    add_edge_graph(&g1,"7","786",1.99607953046,0);
    add_edge_graph(&g1,"7","596",9.47706601923,0);
    add_edge_graph(&g1,"7","473",8.70637334409,0);
    add_edge_graph(&g1,"8","379",0.122603892238,0);
    add_edge_graph(&g1,"9","961",6.00189799538,0);
    add_edge_graph(&g1,"9","34",7.9195031174,0);
    add_edge_graph(&g1,"9","35",9.69295296604,0);
    add_edge_graph(&g1,"9","289",3.7408010911,0);
    add_edge_graph(&g1,"9","874",9.3701514595,0);
    add_edge_graph(&g1,"9","139",2.11327824153,0);
    add_edge_graph(&g1,"9","194",1.53022879392,0);
    add_edge_graph(&g1,"9","557",4.27543979763,0);
    add_edge_graph(&g1,"9","786",3.71711184009,0);
    add_edge_graph(&g1,"9","435",6.26775999865,0);
    add_edge_graph(&g1,"9","596",8.81704727762,0);
    add_edge_graph(&g1,"9","566",7.2597917233,0);
    add_edge_graph(&g1,"9","344",1.43610931901,0);
    add_edge_graph(&g1,"9","628",2.72712057093,0);
    add_edge_graph(&g1,"9","71",5.86595236753,0);
    add_edge_graph(&g1,"9","527",1.26953562034,0);
    add_edge_graph(&g1,"9","861",8.71970841353,0);
    add_edge_graph(&g1,"9","890",3.90542234696,0);
    add_edge_graph(&g1,"9","31",3.00546546036,0);
    add_edge_graph(&g1,"10","645",6.01345524833,0);
    add_edge_graph(&g1,"10","295",9.34490542398,0);
    add_edge_graph(&g1,"10","874",9.61873666896,0);
    add_edge_graph(&g1,"10","46",3.91598722456,0);
    add_edge_graph(&g1,"10","887",0.944012504174,0);
    add_edge_graph(&g1,"10","888",5.34643078615,0);
    add_edge_graph(&g1,"10","121",1.73880957978,0);
    add_edge_graph(&g1,"10","890",6.66557982232,0);
    add_edge_graph(&g1,"10","222",7.95674606013,0);
    add_edge_graph(&g1,"11","850",2.06214078162,0);
    add_edge_graph(&g1,"12","888",0.623272300081,0);
    add_edge_graph(&g1,"12","35",6.06281192434,0);
    add_edge_graph(&g1,"13","888",9.48284945557,0);
    add_edge_graph(&g1,"13","253",1.83899792333,0);
    add_edge_graph(&g1,"14","46",8.77784707717,0);
    add_edge_graph(&g1,"14","527",7.39181853547,0);
    add_edge_graph(&g1,"15","129",7.87030447474,0);
    add_edge_graph(&g1,"15","934",0.480119380029,0);
    add_edge_graph(&g1,"15","745",3.76341383814,0);
    add_edge_graph(&g1,"15","874",3.41268229912,0);
    add_edge_graph(&g1,"15","780",0.0432652262865,0);
    add_edge_graph(&g1,"15","596",0.252160714586,0);
    add_edge_graph(&g1,"15","824",8.61076302561,0);
    add_edge_graph(&g1,"15","890",9.6024832665,0);
    add_edge_graph(&g1,"15","155",4.6862610843,0);
    add_edge_graph(&g1,"15","202",7.97529240282,0);
    add_edge_graph(&g1,"16","248",6.17313363588,0);
    add_edge_graph(&g1,"16","129",4.38061742392,0);
    add_edge_graph(&g1,"17","596",0.307381286709,0);
    add_edge_graph(&g1,"17","595",5.77861960746,0);
    add_edge_graph(&g1,"17","628",6.01954572865,0);
    add_edge_graph(&g1,"18","786",3.87177851493,0);
    add_edge_graph(&g1,"18","139",8.522882373,0);
    add_edge_graph(&g1,"18","154",5.66412070337,0);
    add_edge_graph(&g1,"18","494",0.218113450904,0);
    add_edge_graph(&g1,"18","197",5.92704937774,0);
    add_edge_graph(&g1,"19","596",6.41189477013,0);
    add_edge_graph(&g1,"20","306",8.29121984428,0);
    add_edge_graph(&g1,"20","39",0.482611752973,0);
    add_edge_graph(&g1,"21","129",9.60174027067,0);
    add_edge_graph(&g1,"21","774",8.94517868482,0);
    add_edge_graph(&g1,"21","527",7.1582571112,0);
    add_edge_graph(&g1,"21","786",1.54130922085,0);
    add_edge_graph(&g1,"21","660",2.14877692754,0);
    add_edge_graph(&g1,"21","150",2.49586676545,0);
    add_edge_graph(&g1,"21","794",0.653595756681,0);
    add_edge_graph(&g1,"21","27",4.96359571664,0);
    add_edge_graph(&g1,"21","541",8.36606175707,0);
    add_edge_graph(&g1,"21","672",8.9716326276,0);
    add_edge_graph(&g1,"21","161",2.38247856419,0);
    add_edge_graph(&g1,"21","34",0.724423710024,0);
    add_edge_graph(&g1,"21","35",4.51428878996,0);
    add_edge_graph(&g1,"21","549",5.27886799827,0);
    add_edge_graph(&g1,"21","170",9.83922823455,0);
    add_edge_graph(&g1,"21","811",8.74069520375,0);
    add_edge_graph(&g1,"21","174",1.97212114743,0);
    add_edge_graph(&g1,"21","304",4.80647251156,0);
    add_edge_graph(&g1,"21","689",0.047702954196,0);
    add_edge_graph(&g1,"21","439",2.93267380344,0);
    add_edge_graph(&g1,"21","824",6.47869438976,0);
    add_edge_graph(&g1,"21","186",8.00438878416,0);
    add_edge_graph(&g1,"21","61",6.32170141703,0);
    add_edge_graph(&g1,"21","193",0.772118545068,0);
    add_edge_graph(&g1,"21","289",4.47432060581,0);
    add_edge_graph(&g1,"21","202",3.49486742923,0);
    add_edge_graph(&g1,"21","332",2.17263777446,0);
    add_edge_graph(&g1,"21","81",1.8335124601,0);
    add_edge_graph(&g1,"21","596",4.7211662098,0);
    add_edge_graph(&g1,"21","855",9.30639044351,0);
    add_edge_graph(&g1,"21","476",9.80410909729,0);
    add_edge_graph(&g1,"21","209",9.34249328901,0);
    add_edge_graph(&g1,"21","874",4.76393749968,0);
    add_edge_graph(&g1,"21","359",8.50687148643,0);
    add_edge_graph(&g1,"21","109",5.89636797312,0);
    add_edge_graph(&g1,"21","498",6.92976805273,0);
    add_edge_graph(&g1,"21","628",5.68009409097,0);
    add_edge_graph(&g1,"21","502",0.381841762879,0);
    add_edge_graph(&g1,"21","120",4.48283559671,0);
    add_edge_graph(&g1,"21","890",0.024967435324,0);
    add_edge_graph(&g1,"21","382",1.37691213642,0);
    add_edge_graph(&g1,"21","127",5.01871767197,0);
    add_edge_graph(&g1,"22","35",4.28440402299,0);
    add_edge_graph(&g1,"23","888",7.73742994732,0);
    add_edge_graph(&g1,"23","753",2.07929321126,0);
    add_edge_graph(&g1,"23","35",3.3090074949,0);
    add_edge_graph(&g1,"23","853",1.69912216238,0);
    add_edge_graph(&g1,"23","366",8.22635804258,0);
    add_edge_graph(&g1,"24","890",9.56527079459,0);
    add_edge_graph(&g1,"24","874",4.36005038873,0);
    add_edge_graph(&g1,"24","47",6.86403076534,0);
    add_edge_graph(&g1,"25","890",6.71135545508,0);
    add_edge_graph(&g1,"25","494",7.30554734524,0);
    add_edge_graph(&g1,"26","342",4.37144018987,0);
    add_edge_graph(&g1,"27","392",9.19755480936,0);
    add_edge_graph(&g1,"27","527",9.23805505323,0);
    add_edge_graph(&g1,"27","786",1.19918350143,0);
    add_edge_graph(&g1,"27","289",7.55945273932,0);
    add_edge_graph(&g1,"27","675",3.91920088124,0);
    add_edge_graph(&g1,"27","170",5.65625130152,0);
    add_edge_graph(&g1,"27","427",0.126674787567,0);
    add_edge_graph(&g1,"27","557",4.46515370114,0);
    add_edge_graph(&g1,"27","174",2.55491458461,0);
    add_edge_graph(&g1,"27","954",3.20203623145,0);
    add_edge_graph(&g1,"27","193",1.17741961699,0);
    add_edge_graph(&g1,"27","709",2.99413806814,0);
    add_edge_graph(&g1,"27","74",5.10438461918,0);
    add_edge_graph(&g1,"27","845",9.73612649026,0);
    add_edge_graph(&g1,"27","120",6.18435234209,0);
    add_edge_graph(&g1,"27","858",5.27914513345,0);
    add_edge_graph(&g1,"27","874",0.862841495385,0);
    add_edge_graph(&g1,"27","755",3.85042818673,0);
    add_edge_graph(&g1,"27","502",9.53200533525,0);
    add_edge_graph(&g1,"27","888",6.27858846278,0);
    add_edge_graph(&g1,"27","890",9.52856002825,0);
    add_edge_graph(&g1,"28","290",3.65079342144,0);
    add_edge_graph(&g1,"28","203",2.15203426689,0);
    add_edge_graph(&g1,"29","890",7.22997096776,0);
    add_edge_graph(&g1,"29","470",8.60334695612,0);
    add_edge_graph(&g1,"30","585",2.47350547923,0);
    add_edge_graph(&g1,"30","170",0.968827111821,0);
    add_edge_graph(&g1,"30","596",4.35630183621,0);
    add_edge_graph(&g1,"30","821",1.80264094453,0);
    add_edge_graph(&g1,"30","121",8.9932544215,0);
    add_edge_graph(&g1,"31","182",6.98727117498,0);
    add_edge_graph(&g1,"31","231",2.624341367,0);
    add_edge_graph(&g1,"31","874",2.07520983008,0);
    add_edge_graph(&g1,"31","45",7.14762997755,0);
    add_edge_graph(&g1,"31","79",8.27677457728,0);
    add_edge_graph(&g1,"31","176",4.45123447443,0);
    add_edge_graph(&g1,"31","502",4.67019772988,0);
    add_edge_graph(&g1,"31","73",7.46126831703,0);
    add_edge_graph(&g1,"31","890",8.36660739522,0);
    add_edge_graph(&g1,"31","170",4.574201772,0);
    add_edge_graph(&g1,"32","786",4.67079385918,0);
    add_edge_graph(&g1,"32","35",5.3427483552,0);
    add_edge_graph(&g1,"32","202",3.62738469794,0);
    add_edge_graph(&g1,"32","205",2.70906533324,0);
    add_edge_graph(&g1,"33","890",7.68025942245,0);
    add_edge_graph(&g1,"34","391",9.85035649158,0);
    add_edge_graph(&g1,"34","139",2.4226877288,0);
    add_edge_graph(&g1,"34","910",5.241329816,0);
    add_edge_graph(&g1,"34","527",6.33052488038,0);
    add_edge_graph(&g1,"34","786",4.00928228692,0);
    add_edge_graph(&g1,"34","170",9.96540065472,0);
    add_edge_graph(&g1,"34","427",1.97249787549,0);
    add_edge_graph(&g1,"34","888",4.63092856362,0);
    add_edge_graph(&g1,"34","52",0.929671591778,0);
    add_edge_graph(&g1,"34","192",0.76083990654,0);
    add_edge_graph(&g1,"34","705",6.15695696773,0);
    add_edge_graph(&g1,"34","451",6.81826366984,0);
    add_edge_graph(&g1,"34","202",5.05194343491,0);
    add_edge_graph(&g1,"34","205",8.46840627595,0);
    add_edge_graph(&g1,"34","81",0.457001601561,0);
    add_edge_graph(&g1,"34","470",8.31633914179,0);
    add_edge_graph(&g1,"34","863",6.17980794159,0);
    add_edge_graph(&g1,"34","744",0.706942312072,0);
    add_edge_graph(&g1,"34","874",8.0492453818,0);
    add_edge_graph(&g1,"34","111",0.373413064544,0);
    add_edge_graph(&g1,"34","244",0.480561534853,0);
    add_edge_graph(&g1,"34","502",8.82170249723,0);
    add_edge_graph(&g1,"34","120",1.26354006267,0);
    add_edge_graph(&g1,"34","234",6.73721072052,0);
    add_edge_graph(&g1,"34","251",9.60079622544,0);
    add_edge_graph(&g1,"34","253",3.00523280241,0);
    add_edge_graph(&g1,"34","511",8.82192454503,0);
    add_edge_graph(&g1,"35","518",1.76935700726,0);
    add_edge_graph(&g1,"35","521",7.60582073981,0);
    add_edge_graph(&g1,"35","527",1.60066040884,0);
    add_edge_graph(&g1,"35","531",2.04092019814,0);
    add_edge_graph(&g1,"35","39",7.2565828517,0);
    add_edge_graph(&g1,"35","44",9.04597928583,0);
    add_edge_graph(&g1,"35","557",4.58772820257,0);
    add_edge_graph(&g1,"35","46",4.08018507063,0);
    add_edge_graph(&g1,"35","47",1.8587759739,0);
    add_edge_graph(&g1,"35","585",6.77549362798,0);
    add_edge_graph(&g1,"35","588",6.38044193745,0);
    add_edge_graph(&g1,"35","78",2.55762749668,0);
    add_edge_graph(&g1,"35","79",2.73006573366,0);
    add_edge_graph(&g1,"35","81",9.2878674389,0);
    add_edge_graph(&g1,"35","596",6.99091141011,0);
    add_edge_graph(&g1,"35","598",2.29843298457,0);
    add_edge_graph(&g1,"35","97",8.69833201925,0);
    add_edge_graph(&g1,"35","624",7.79177150892,0);
    add_edge_graph(&g1,"35","628",8.3181274053,0);
    add_edge_graph(&g1,"35","120",2.9539516069,0);
    add_edge_graph(&g1,"35","129",0.770822116493,0);
    add_edge_graph(&g1,"35","642",3.18363888615,0);
    add_edge_graph(&g1,"35","139",5.25155320505,0);
    add_edge_graph(&g1,"35","150",1.17184118764,0);
    add_edge_graph(&g1,"35","152",1.02866589852,0);
    add_edge_graph(&g1,"35","154",5.26122282191,0);
    add_edge_graph(&g1,"35","155",4.64826640634,0);
    add_edge_graph(&g1,"35","675",1.49916826608,0);
    add_edge_graph(&g1,"35","170",3.85951452073,0);
    add_edge_graph(&g1,"35","174",1.23508515669,0);
    add_edge_graph(&g1,"35","689",2.47282222959,0);
    add_edge_graph(&g1,"35","697",7.15342720651,0);
    add_edge_graph(&g1,"35","190",5.47612641866,0);
    add_edge_graph(&g1,"35","544",3.82135762958,0);
    add_edge_graph(&g1,"35","197",9.67096392462,0);
    add_edge_graph(&g1,"35","198",1.79724791778,0);
    add_edge_graph(&g1,"35","712",3.5811521078,0);
    add_edge_graph(&g1,"35","202",2.61386153548,0);
    add_edge_graph(&g1,"35","205",8.88151296884,0);
    add_edge_graph(&g1,"35","206",5.55473208489,0);
    add_edge_graph(&g1,"35","725",1.34145126453,0);
    add_edge_graph(&g1,"35","220",8.51855474055,0);
    add_edge_graph(&g1,"35","226",1.41399314809,0);
    add_edge_graph(&g1,"35","230",4.38426598156,0);
    add_edge_graph(&g1,"35","743",6.94332030795,0);
    add_edge_graph(&g1,"35","745",6.25013754304,0);
    add_edge_graph(&g1,"35","761",1.28563491787,0);
    add_edge_graph(&g1,"35","251",2.75981041628,0);
    add_edge_graph(&g1,"35","262",8.64963784169,0);
    add_edge_graph(&g1,"35","780",4.37882520787,0);
    add_edge_graph(&g1,"35","273",5.98154790782,0);
    add_edge_graph(&g1,"35","786",6.57880220896,0);
    add_edge_graph(&g1,"35","787",9.02848993704,0);
    add_edge_graph(&g1,"35","795",0.871354400774,0);
    add_edge_graph(&g1,"35","797",2.6498379767,0);
    add_edge_graph(&g1,"35","289",5.7859403387,0);
    add_edge_graph(&g1,"35","293",7.62207902161,0);
    add_edge_graph(&g1,"35","295",6.5039808101,0);
    add_edge_graph(&g1,"35","810",2.76631845272,0);
    add_edge_graph(&g1,"35","304",4.44551236512,0);
    add_edge_graph(&g1,"35","824",3.9324939245,0);
    add_edge_graph(&g1,"35","321",7.99664400346,0);
    add_edge_graph(&g1,"35","322",4.62035118942,0);
    add_edge_graph(&g1,"35","843",0.76310222015,0);
    add_edge_graph(&g1,"35","335",8.16347965152,0);
    add_edge_graph(&g1,"35","854",2.92238551988,0);
    add_edge_graph(&g1,"35","350",3.55114788478,0);
    add_edge_graph(&g1,"35","359",6.06579678356,0);
    add_edge_graph(&g1,"35","874",8.52263710711,0);
    add_edge_graph(&g1,"35","888",6.592153199,0);
    add_edge_graph(&g1,"35","890",4.69709982477,0);
    add_edge_graph(&g1,"35","380",0.992439336146,0);
    add_edge_graph(&g1,"35","392",2.69203603843,0);
    add_edge_graph(&g1,"35","915",5.59524566892,0);
    add_edge_graph(&g1,"35","409",7.80032532939,0);
    add_edge_graph(&g1,"35","423",3.99314054752,0);
    add_edge_graph(&g1,"35","432",5.72021759327,0);
    add_edge_graph(&g1,"35","948",0.829967317136,0);
    add_edge_graph(&g1,"35","950",8.05494504366,0);
    add_edge_graph(&g1,"35","444",0.108969514461,0);
    add_edge_graph(&g1,"35","446",3.19864204913,0);
    add_edge_graph(&g1,"35","961",2.84784069083,0);
    add_edge_graph(&g1,"35","450",7.52383390422,0);
    add_edge_graph(&g1,"35","453",6.25434391069,0);
    add_edge_graph(&g1,"35","455",4.42300357744,0);
    add_edge_graph(&g1,"35","968",9.06389083701,0);
    add_edge_graph(&g1,"35","457",6.67742748839,0);
    add_edge_graph(&g1,"35","472",4.12551306603,0);
    add_edge_graph(&g1,"35","473",2.71991645533,0);
    add_edge_graph(&g1,"35","474",8.88069304008,0);
    add_edge_graph(&g1,"35","997",6.27883331485,0);
    add_edge_graph(&g1,"35","593",5.53352827247,0);
    add_edge_graph(&g1,"35","495",0.469075474662,0);
    add_edge_graph(&g1,"35","499",6.81790091727,0);
    add_edge_graph(&g1,"36","129",2.80403259149,0);
    add_edge_graph(&g1,"36","388",4.64277080674,0);
    add_edge_graph(&g1,"36","139",0.0628220453695,0);
    add_edge_graph(&g1,"36","527",5.82368196441,0);
    add_edge_graph(&g1,"36","144",3.22011674041,0);
    add_edge_graph(&g1,"36","786",4.60756755535,0);
    add_edge_graph(&g1,"36","535",7.39272067421,0);
    add_edge_graph(&g1,"36","155",1.37401700547,0);
    add_edge_graph(&g1,"36","414",9.43244656418,0);
    add_edge_graph(&g1,"36","289",0.325100568891,0);
    add_edge_graph(&g1,"36","418",5.56065484257,0);
    add_edge_graph(&g1,"36","164",2.30771211144,0);
    add_edge_graph(&g1,"36","295",0.139820469118,0);
    add_edge_graph(&g1,"36","170",3.80724142682,0);
    add_edge_graph(&g1,"36","46",5.69770252495,0);
    add_edge_graph(&g1,"36","432",1.98090403392,0);
    add_edge_graph(&g1,"36","824",5.11097757636,0);
    add_edge_graph(&g1,"36","317",6.52705263249,0);
    add_edge_graph(&g1,"36","703",3.044027419,0);
    add_edge_graph(&g1,"36","578",5.8654646042,0);
    add_edge_graph(&g1,"36","835",0.462049871133,0);
    add_edge_graph(&g1,"36","457",6.7749914191,0);
    add_edge_graph(&g1,"36","202",3.23467914638,0);
    add_edge_graph(&g1,"36","938",9.92583817198,0);
    add_edge_graph(&g1,"36","205",3.94694618709,0);
    add_edge_graph(&g1,"36","974",0.683035544971,0);
    add_edge_graph(&g1,"36","81",5.94429036254,0);
    add_edge_graph(&g1,"36","596",0.145164967737,0);
    add_edge_graph(&g1,"36","471",0.453260059883,0);
    add_edge_graph(&g1,"36","474",6.8138068487,0);
    add_edge_graph(&g1,"36","732",1.60845811951,0);
    add_edge_graph(&g1,"36","97",6.29243324818,0);
    add_edge_graph(&g1,"36","744",8.33181370477,0);
    add_edge_graph(&g1,"36","874",3.5562358612,0);
    add_edge_graph(&g1,"36","39",8.53544937322,0);
    add_edge_graph(&g1,"36","366",3.76167051064,0);
    add_edge_graph(&g1,"36","765",7.02561286257,0);
    add_edge_graph(&g1,"36","625",5.15918122406,0);
    add_edge_graph(&g1,"36","244",4.00317577779,0);
    add_edge_graph(&g1,"36","117",5.81335799297,0);
    add_edge_graph(&g1,"36","888",0.741130897477,0);
    add_edge_graph(&g1,"36","890",5.94287697386,0);
    add_edge_graph(&g1,"36","381",8.72917492203,0);
    add_edge_graph(&g1,"37","872",9.22063331926,0);
    add_edge_graph(&g1,"38","888",1.90644916157,0);
    add_edge_graph(&g1,"39","385",9.331155027,0);
    add_edge_graph(&g1,"39","900",6.14858377662,0);
    add_edge_graph(&g1,"39","389",4.49279336133,0);
    add_edge_graph(&g1,"39","774",9.83390964162,0);
    add_edge_graph(&g1,"39","129",7.00712133012,0);
    add_edge_graph(&g1,"39","789",5.68093939891,0);
    add_edge_graph(&g1,"39","795",3.17009787509,0);
    add_edge_graph(&g1,"39","170",9.31893718833,0);
    add_edge_graph(&g1,"39","686",7.34700593491,0);
    add_edge_graph(&g1,"39","697",2.20797029882,0);
    add_edge_graph(&g1,"39","963",7.20535572061,0);
    add_edge_graph(&g1,"39","327",8.73085251072,0);
    add_edge_graph(&g1,"39","714",7.23154475065,0);
    add_edge_graph(&g1,"39","79",6.61344274259,0);
    add_edge_graph(&g1,"39","596",7.77839936688,0);
    add_edge_graph(&g1,"39","470",1.70094411263,0);
    add_edge_graph(&g1,"39","761",3.99438678601,0);
    add_edge_graph(&g1,"39","987",5.59787468862,0);
    add_edge_graph(&g1,"39","230",3.25853131368,0);
    add_edge_graph(&g1,"39","238",8.79382318567,0);
    add_edge_graph(&g1,"39","888",5.38010359608,0);
    add_edge_graph(&g1,"39","121",6.76534025625,0);
    add_edge_graph(&g1,"39","890",6.10511858542,0);
    add_edge_graph(&g1,"40","97",0.543141016951,0);
    add_edge_graph(&g1,"40","874",9.6932901649,0);
    add_edge_graph(&g1,"40","139",6.32922202458,0);
    add_edge_graph(&g1,"40","273",1.76957281986,0);
    add_edge_graph(&g1,"41","964",2.94074756798,0);
    add_edge_graph(&g1,"42","129",0.220862846525,0);
    add_edge_graph(&g1,"42","155",6.74573324478,0);
    add_edge_graph(&g1,"42","772",3.23921713348,0);
    add_edge_graph(&g1,"42","150",3.00629515018,0);
    add_edge_graph(&g1,"42","294",0.141360208927,0);
    add_edge_graph(&g1,"42","874",7.00309937692,0);
    add_edge_graph(&g1,"42","523",9.21210398973,0);
    add_edge_graph(&g1,"42","527",1.77514216465,0);
    add_edge_graph(&g1,"42","785",0.762154252477,0);
    add_edge_graph(&g1,"42","786",0.0165878901389,0);
    add_edge_graph(&g1,"42","212",6.53973025839,0);
    add_edge_graph(&g1,"42","278",4.25407525106,0);
    add_edge_graph(&g1,"42","535",9.12457214889,0);
    add_edge_graph(&g1,"42","890",7.2944179436,0);
    add_edge_graph(&g1,"42","91",1.61638526208,0);
    add_edge_graph(&g1,"43","704",4.47536755489,0);
    add_edge_graph(&g1,"43","596",8.40433398196,0);
    add_edge_graph(&g1,"43","236",1.7219348115,0);
    add_edge_graph(&g1,"43","774",8.32409368428,0);
    add_edge_graph(&g1,"44","129",4.03853151295,0);
    add_edge_graph(&g1,"44","450",8.8535142133,0);
    add_edge_graph(&g1,"44","516",3.16250373466,0);
    add_edge_graph(&g1,"44","776",1.41009423375,0);
    add_edge_graph(&g1,"44","745",8.01979591791,0);
    add_edge_graph(&g1,"44","427",7.23390489223,0);
    add_edge_graph(&g1,"44","270",5.38775815216,0);
    add_edge_graph(&g1,"44","401",0.480967722698,0);
    add_edge_graph(&g1,"44","786",9.6939678101,0);
    add_edge_graph(&g1,"44","502",4.07758032646,0);
    add_edge_graph(&g1,"44","473",1.58603454491,0);
    add_edge_graph(&g1,"44","58",1.70671153963,0);
    add_edge_graph(&g1,"44","890",3.46747364819,0);
    add_edge_graph(&g1,"45","193",6.22195484359,0);
    add_edge_graph(&g1,"46","129",0.496938546573,0);
    add_edge_graph(&g1,"46","898",2.66823707286,0);
    add_edge_graph(&g1,"46","389",0.864002103818,0);
    add_edge_graph(&g1,"46","774",1.77865967789,0);
    add_edge_graph(&g1,"46","961",8.46138144855,0);
    add_edge_graph(&g1,"46","139",5.20634924693,0);
    add_edge_graph(&g1,"46","780",6.80910042996,0);
    add_edge_graph(&g1,"46","450",8.74931506973,0);
    add_edge_graph(&g1,"46","527",1.78137105021,0);
    add_edge_graph(&g1,"46","528",8.83942238936,0);
    add_edge_graph(&g1,"46","273",0.262612500427,0);
    add_edge_graph(&g1,"46","786",7.5335189782,0);
    add_edge_graph(&g1,"46","403",0.153685820689,0);
    add_edge_graph(&g1,"46","151",4.67244069752,0);
    add_edge_graph(&g1,"46","120",8.53123129096,0);
    add_edge_graph(&g1,"46","154",3.84956738272,0);
    add_edge_graph(&g1,"46","795",2.17894660535,0);
    add_edge_graph(&g1,"46","670",4.10007256779,0);
    add_edge_graph(&g1,"46","517",1.50623006127,0);
    add_edge_graph(&g1,"46","930",7.57037198323,0);
    add_edge_graph(&g1,"46","518",5.6957449746,0);
    add_edge_graph(&g1,"46","295",6.29178411548,0);
    add_edge_graph(&g1,"46","170",3.46870651741,0);
    add_edge_graph(&g1,"46","171",0.457699664654,0);
    add_edge_graph(&g1,"46","940",7.83428689673,0);
    add_edge_graph(&g1,"46","685",7.38162538132,0);
    add_edge_graph(&g1,"46","504",2.75137164478,0);
    add_edge_graph(&g1,"46","547",8.3685427062,0);
    add_edge_graph(&g1,"46","182",5.1529014428,0);
    add_edge_graph(&g1,"46","823",7.65014807947,0);
    add_edge_graph(&g1,"46","283",3.73362000419,0);
    add_edge_graph(&g1,"46","186",6.29051648886,0);
    add_edge_graph(&g1,"46","572",1.58231257154,0);
    add_edge_graph(&g1,"46","193",5.09700491615,0);
    add_edge_graph(&g1,"46","322",4.68109095385,0);
    add_edge_graph(&g1,"46","835",2.78494812528,0);
    add_edge_graph(&g1,"46","324",0.696291511656,0);
    add_edge_graph(&g1,"46","950",4.85658681102,0);
    add_edge_graph(&g1,"46","211",4.06253282644,0);
    add_edge_graph(&g1,"46","327",9.71029858476,0);
    add_edge_graph(&g1,"46","73",6.71163419312,0);
    add_edge_graph(&g1,"46","202",4.25633967035,0);
    add_edge_graph(&g1,"46","205",3.7248568706,0);
    add_edge_graph(&g1,"46","718",2.12874389032,0);
    add_edge_graph(&g1,"46","888",7.26998778169,0);
    add_edge_graph(&g1,"46","557",8.50354633178,0);
    add_edge_graph(&g1,"46","675",4.64065320724,0);
    add_edge_graph(&g1,"46","596",5.7340104377,0);
    add_edge_graph(&g1,"46","469",5.46611866804,0);
    add_edge_graph(&g1,"46","473",7.16823473842,0);
    add_edge_graph(&g1,"46","474",5.09845958263,0);
    add_edge_graph(&g1,"46","603",6.21410985055,0);
    add_edge_graph(&g1,"46","476",3.24423291587,0);
    add_edge_graph(&g1,"46","349",4.46262216172,0);
    add_edge_graph(&g1,"46","736",0.719015508833,0);
    add_edge_graph(&g1,"46","419",4.76246251111,0);
    add_edge_graph(&g1,"46","611",1.5324314117,0);
    add_edge_graph(&g1,"46","455",5.60639843611,0);
    add_edge_graph(&g1,"46","744",0.126962671684,0);
    add_edge_graph(&g1,"46","828",7.05881443781,0);
    add_edge_graph(&g1,"46","874",4.53273740832,0);
    add_edge_graph(&g1,"46","745",8.25206208265,0);
    add_edge_graph(&g1,"46","109",3.945900315,0);
    add_edge_graph(&g1,"46","625",5.4002797337,0);
    add_edge_graph(&g1,"46","531",2.81149155747,0);
    add_edge_graph(&g1,"46","886",6.60242547135,0);
    add_edge_graph(&g1,"46","247",9.5392892377,0);
    add_edge_graph(&g1,"46","376",1.54596116067,0);
    add_edge_graph(&g1,"46","890",5.75092424024,0);
    add_edge_graph(&g1,"46","810",0.879377127171,0);
    add_edge_graph(&g1,"47","704",8.16792449234,0);
    add_edge_graph(&g1,"47","129",7.53306166081,0);
    add_edge_graph(&g1,"47","738",4.90233043711,0);
    add_edge_graph(&g1,"47","325",3.17546953148,0);
    add_edge_graph(&g1,"47","253",3.66454064246,0);
    add_edge_graph(&g1,"47","104",3.4701691236,0);
    add_edge_graph(&g1,"47","874",1.67269678926,0);
    add_edge_graph(&g1,"47","527",0.686454431347,0);
    add_edge_graph(&g1,"47","824",9.2111528676,0);
    add_edge_graph(&g1,"47","786",8.28354027317,0);
    add_edge_graph(&g1,"47","739",6.14383892289,0);
    add_edge_graph(&g1,"47","244",0.128905597074,0);
    add_edge_graph(&g1,"47","573",9.70047120647,0);
    add_edge_graph(&g1,"47","669",0.513489787929,0);
    add_edge_graph(&g1,"48","616",7.7158143938,0);
    add_edge_graph(&g1,"49","322",5.2236476163,0);
    add_edge_graph(&g1,"49","874",0.195497392505,0);
    add_edge_graph(&g1,"49","500",4.30094296854,0);
    add_edge_graph(&g1,"49","120",0.512337048848,0);
    add_edge_graph(&g1,"49","253",9.46633044819,0);
    add_edge_graph(&g1,"49","511",0.727535792872,0);
    add_edge_graph(&g1,"50","129",5.25219190992,0);
    add_edge_graph(&g1,"51","795",8.02803585483,0);
    add_edge_graph(&g1,"51","596",4.62370830843,0);
    add_edge_graph(&g1,"52","547",5.88160526641,0);
    add_edge_graph(&g1,"52","139",8.8478312532,0);
    add_edge_graph(&g1,"52","317",2.33350138696,0);
    add_edge_graph(&g1,"52","271",6.24326001423,0);
    add_edge_graph(&g1,"53","890",1.80949579164,0);
    add_edge_graph(&g1,"53","119",4.95048339175,0);
    add_edge_graph(&g1,"53","151",9.26286849537,0);
    add_edge_graph(&g1,"54","78",8.5089129082,0);
    add_edge_graph(&g1,"55","888",1.66394903971,0);
    add_edge_graph(&g1,"55","752",9.06971398434,0);
    add_edge_graph(&g1,"55","139",9.7705476282,0);
    add_edge_graph(&g1,"56","888",9.13712091062,0);
    add_edge_graph(&g1,"56","874",0.185223096105,0);
    add_edge_graph(&g1,"56","810",6.67179343013,0);
    add_edge_graph(&g1,"56","493",3.9722518998,0);
    add_edge_graph(&g1,"56","749",9.06815844541,0);
    add_edge_graph(&g1,"57","596",4.84271643094,0);
    add_edge_graph(&g1,"57","874",2.37494384487,0);
    add_edge_graph(&g1,"57","556",9.39405794949,0);
    add_edge_graph(&g1,"57","202",6.87884413802,0);
    add_edge_graph(&g1,"58","874",8.69544562612,0);
    add_edge_graph(&g1,"59","139",7.00808274165,0);
    add_edge_graph(&g1,"59","628",5.99094722352,0);
    add_edge_graph(&g1,"60","890",6.45485188082,0);
    add_edge_graph(&g1,"61","174",6.92005297793,0);
    add_edge_graph(&g1,"62","227",9.68106943123,0);
    add_edge_graph(&g1,"62","774",9.47441948886,0);
    add_edge_graph(&g1,"62","170",0.939111877438,0);
    add_edge_graph(&g1,"62","139",4.29631264487,0);
    add_edge_graph(&g1,"62","780",5.76399167778,0);
    add_edge_graph(&g1,"62","890",5.35355818448,0);
    add_edge_graph(&g1,"62","874",1.76855701789,0);
    add_edge_graph(&g1,"63","891",6.1446917813,0);
    add_edge_graph(&g1,"64","874",9.57644279043,0);
    add_edge_graph(&g1,"64","596",6.96820455292,0);
    add_edge_graph(&g1,"65","332",5.542590316,0);
    add_edge_graph(&g1,"66","515",7.31412521946,0);
    add_edge_graph(&g1,"66","174",8.93175839424,0);
    add_edge_graph(&g1,"67","874",7.29245932785,0);
    add_edge_graph(&g1,"67","890",7.89657622617,0);
    add_edge_graph(&g1,"67","182",4.03215908745,0);
    add_edge_graph(&g1,"67","170",7.73986651953,0);
    add_edge_graph(&g1,"68","176",5.92425990557,0);
    add_edge_graph(&g1,"68","635",9.57417803562,0);
    add_edge_graph(&g1,"68","557",4.59479295088,0);
    add_edge_graph(&g1,"69","865",2.29982305966,0);
    add_edge_graph(&g1,"69","244",8.91699539669,0);
    add_edge_graph(&g1,"70","888",4.47985127116,0);
    add_edge_graph(&g1,"70","890",8.07112219173,0);
    add_edge_graph(&g1,"71","691",0.174294576058,0);
    add_edge_graph(&g1,"72","129",5.95362290779,0);
    add_edge_graph(&g1,"72","596",0.406275149834,0);
    add_edge_graph(&g1,"73","290",5.50709919132,0);
    add_edge_graph(&g1,"73","547",4.31111562633,0);
    add_edge_graph(&g1,"73","772",0.61768617186,0);
    add_edge_graph(&g1,"73","453",8.50023946331,0);
    add_edge_graph(&g1,"73","423",8.7279888838,0);
    add_edge_graph(&g1,"73","170",1.83508379164,0);
    add_edge_graph(&g1,"73","205",5.99963303117,0);
    add_edge_graph(&g1,"73","752",0.975184853092,0);
    add_edge_graph(&g1,"73","212",8.56146618051,0);
    add_edge_graph(&g1,"73","888",6.34799136512,0);
    add_edge_graph(&g1,"73","473",7.07733563958,0);
    add_edge_graph(&g1,"73","890",7.04184831229,0);
    add_edge_graph(&g1,"73","874",5.35213170087,0);
    add_edge_graph(&g1,"74","692",9.24980703497,0);
    add_edge_graph(&g1,"75","129",7.97351018607,0);
    add_edge_graph(&g1,"76","963",2.24030839865,0);
    add_edge_graph(&g1,"77","129",1.11369069448,0);
    add_edge_graph(&g1,"77","801",4.29333257361,0);
    add_edge_graph(&g1,"78","205",9.24188166194,0);
    add_edge_graph(&g1,"78","527",9.02542582943,0);
    add_edge_graph(&g1,"78","890",8.92256516024,0);
    add_edge_graph(&g1,"78","507",2.77961865146,0);
    add_edge_graph(&g1,"79","129",0.317837745847,0);
    add_edge_graph(&g1,"79","193",7.47709594499,0);
    add_edge_graph(&g1,"79","139",2.66613861511,0);
    add_edge_graph(&g1,"79","397",9.62843163013,0);
    add_edge_graph(&g1,"79","527",6.23115229755,0);
    add_edge_graph(&g1,"79","530",7.16914858072,0);
    add_edge_graph(&g1,"79","406",1.71453569226,0);
    add_edge_graph(&g1,"79","151",2.95384631302,0);
    add_edge_graph(&g1,"79","408",6.16939190364,0);
    add_edge_graph(&g1,"79","795",9.54909496731,0);
    add_edge_graph(&g1,"79","687",2.39971892821,0);
    add_edge_graph(&g1,"79","159",9.84759889144,0);
    add_edge_graph(&g1,"79","289",8.61424928414,0);
    add_edge_graph(&g1,"79","550",9.42447470157,0);
    add_edge_graph(&g1,"79","476",2.41759754875,0);
    add_edge_graph(&g1,"79","170",2.59428958382,0);
    add_edge_graph(&g1,"79","174",8.26210596746,0);
    add_edge_graph(&g1,"79","303",5.61069706929,0);
    add_edge_graph(&g1,"79","547",2.65215002781,0);
    add_edge_graph(&g1,"79","178",7.55617903304,0);
    add_edge_graph(&g1,"79","950",9.97263404879,0);
    add_edge_graph(&g1,"79","824",7.63089627256,0);
    add_edge_graph(&g1,"79","318",7.76780465913,0);
    add_edge_graph(&g1,"79","961",0.926059750849,0);
    add_edge_graph(&g1,"79","579",1.23739029469,0);
    add_edge_graph(&g1,"79","453",0.806006765677,0);
    add_edge_graph(&g1,"79","839",2.45476643322,0);
    add_edge_graph(&g1,"79","202",7.00769679243,0);
    add_edge_graph(&g1,"79","675",3.58027850418,0);
    add_edge_graph(&g1,"79","596",5.23230699134,0);
    add_edge_graph(&g1,"79","723",6.80455175057,0);
    add_edge_graph(&g1,"79","216",6.5620359156,0);
    add_edge_graph(&g1,"79","473",3.15637236739,0);
    add_edge_graph(&g1,"79","348",3.31630571631,0);
    add_edge_graph(&g1,"79","739",0.461914030242,0);
    add_edge_graph(&g1,"79","358",5.31001620098,0);
    add_edge_graph(&g1,"79","874",0.601615126372,0);
    add_edge_graph(&g1,"79","878",0.904656814744,0);
    add_edge_graph(&g1,"79","211",4.17282243991,0);
    add_edge_graph(&g1,"79","888",1.32053018893,0);
    add_edge_graph(&g1,"79","890",7.78251017016,0);
    add_edge_graph(&g1,"79","253",5.50012475519,0);
    add_edge_graph(&g1,"80","596",3.25827733826,0);
    add_edge_graph(&g1,"81","129",3.27385092055,0);
    add_edge_graph(&g1,"81","139",0.153815601443,0);
    add_edge_graph(&g1,"81","268",1.2136790059,0);
    add_edge_graph(&g1,"81","527",9.2670945415,0);
    add_edge_graph(&g1,"81","918",8.37361153228,0);
    add_edge_graph(&g1,"81","154",1.10098996845,0);
    add_edge_graph(&g1,"81","796",8.93114071775,0);
    add_edge_graph(&g1,"81","170",3.14750788059,0);
    add_edge_graph(&g1,"81","427",7.59513619791,0);
    add_edge_graph(&g1,"81","685",8.07974366589,0);
    add_edge_graph(&g1,"81","174",7.84540081831,0);
    add_edge_graph(&g1,"81","950",5.24538564705,0);
    add_edge_graph(&g1,"81","500",8.85691571168,0);
    add_edge_graph(&g1,"81","970",1.5092010907,0);
    add_edge_graph(&g1,"81","205",3.00239816433,0);
    add_edge_graph(&g1,"81","207",1.56711134217,0);
    add_edge_graph(&g1,"81","209",2.1381827508,0);
    add_edge_graph(&g1,"81","547",9.4275052236,0);
    add_edge_graph(&g1,"81","596",5.85528817523,0);
    add_edge_graph(&g1,"81","988",8.70776404001,0);
    add_edge_graph(&g1,"81","989",7.72436039416,0);
    add_edge_graph(&g1,"81","607",1.13156497032,0);
    add_edge_graph(&g1,"81","874",8.92711942627,0);
    add_edge_graph(&g1,"81","366",7.64314000409,0);
    add_edge_graph(&g1,"81","244",3.09537772167,0);
    add_edge_graph(&g1,"81","890",2.93598321132,0);
    add_edge_graph(&g1,"82","596",2.24819733747,0);
    add_edge_graph(&g1,"82","476",5.96816752131,0);
    add_edge_graph(&g1,"83","120",5.5074836272,0);
    add_edge_graph(&g1,"83","964",8.33838380471,0);
    add_edge_graph(&g1,"84","808",8.95382177978,0);
    add_edge_graph(&g1,"85","258",2.17894158236,0);
    add_edge_graph(&g1,"85","890",3.10819992236,0);
    add_edge_graph(&g1,"85","147",9.50527368326,0);
    add_edge_graph(&g1,"85","786",8.89243020222,0);
    add_edge_graph(&g1,"85","174",4.746436896,0);
    add_edge_graph(&g1,"86","139",9.69776594027,0);
    add_edge_graph(&g1,"87","804",0.136245393741,0);
    add_edge_graph(&g1,"87","522",6.0431251135,0);
    add_edge_graph(&g1,"87","812",0.922781037004,0);
    add_edge_graph(&g1,"87","210",7.23059904896,0);
    add_edge_graph(&g1,"87","244",7.91539798999,0);
    add_edge_graph(&g1,"87","120",9.06121090874,0);
    add_edge_graph(&g1,"87","703",5.84988108654,0);
    add_edge_graph(&g1,"88","268",6.19859469914,0);
    add_edge_graph(&g1,"89","596",7.14602875129,0);
    add_edge_graph(&g1,"90","193",9.21469700958,0);
    add_edge_graph(&g1,"90","890",7.05179915059,0);
    add_edge_graph(&g1,"90","139",9.63758507465,0);
    add_edge_graph(&g1,"90","581",8.17396403822,0);
    add_edge_graph(&g1,"91","596",6.09972934499,0);
    add_edge_graph(&g1,"92","129",4.10573821008,0);
    add_edge_graph(&g1,"92","145",7.70157117609,0);
    add_edge_graph(&g1,"93","139",7.27504959934,0);
    add_edge_graph(&g1,"94","733",0.69153182835,0);
    add_edge_graph(&g1,"95","366",7.61862310928,0);
    add_edge_graph(&g1,"96","874",0.459962142172,0);
    add_edge_graph(&g1,"96","142",5.92008179428,0);
    add_edge_graph(&g1,"97","229",0.430870751944,0);
    add_edge_graph(&g1,"97","583",0.493640784998,0);
    add_edge_graph(&g1,"97","139",3.18498818214,0);
    add_edge_graph(&g1,"97","557",1.69248727684,0);
    add_edge_graph(&g1,"97","464",5.95730417237,0);
    add_edge_graph(&g1,"97","467",8.47318858757,0);
    add_edge_graph(&g1,"97","662",0.609190200009,0);
    add_edge_graph(&g1,"97","439",2.67777248742,0);
    add_edge_graph(&g1,"97","216",7.60452047505,0);
    add_edge_graph(&g1,"97","121",6.40867800583,0);
    add_edge_graph(&g1,"97","890",4.64368709082,0);
    add_edge_graph(&g1,"97","476",9.26534185162,0);
    add_edge_graph(&g1,"98","402",5.100629053,0);
    add_edge_graph(&g1,"99","105",3.25580599167,0);
    add_edge_graph(&g1,"100","473",4.33726566113,0);
    add_edge_graph(&g1,"100","845",9.96705426668,0);
    add_edge_graph(&g1,"100","527",4.88766493351,0);
    add_edge_graph(&g1,"101","593",3.49452006431,0);
    add_edge_graph(&g1,"102","331",8.88174026826,0);
    add_edge_graph(&g1,"103","527",8.39920746128,0);
    add_edge_graph(&g1,"104","217",8.39202370498,0);
    add_edge_graph(&g1,"104","890",6.14638597265,0);
    add_edge_graph(&g1,"104","295",5.82283190358,0);
    add_edge_graph(&g1,"104","446",3.23681062908,0);
    add_edge_graph(&g1,"105","647",0.220170449801,0);
    add_edge_graph(&g1,"106","890",4.26254948883,0);
    add_edge_graph(&g1,"106","276",4.11211498571,0);
    add_edge_graph(&g1,"107","366",5.01059145436,0);
    add_edge_graph(&g1,"107","174",6.21902684455,0);
    add_edge_graph(&g1,"107","527",6.33987327954,0);
    add_edge_graph(&g1,"108","874",3.28093551531,0);
    add_edge_graph(&g1,"108","139",9.64316626438,0);
    add_edge_graph(&g1,"108","786",9.28037291704,0);
    add_edge_graph(&g1,"108","280",1.28237759026,0);
    add_edge_graph(&g1,"108","890",6.09139281521,0);
    add_edge_graph(&g1,"109","732",2.57780789739,0);
    add_edge_graph(&g1,"109","596",9.45504609932,0);
    add_edge_graph(&g1,"110","129",7.93457083863,0);
    add_edge_graph(&g1,"111","992",0.449157600475,0);
    add_edge_graph(&g1,"111","165",6.44074455414,0);
    add_edge_graph(&g1,"111","139",0.377008374014,0);
    add_edge_graph(&g1,"111","366",4.89914943259,0);
    add_edge_graph(&g1,"111","274",9.6276972995,0);
    add_edge_graph(&g1,"111","598",0.963256819615,0);
    add_edge_graph(&g1,"111","120",9.14082665931,0);
    add_edge_graph(&g1,"111","761",2.70917164305,0);
    add_edge_graph(&g1,"111","890",7.43121739963,0);
    add_edge_graph(&g1,"112","530",8.72732357774,0);
    add_edge_graph(&g1,"112","202",4.69997519085,0);
    add_edge_graph(&g1,"113","772",9.94331688524,0);
    add_edge_graph(&g1,"113","874",0.202272859488,0);
    add_edge_graph(&g1,"113","636",1.83773703488,0);
    add_edge_graph(&g1,"113","786",5.97486252713,0);
    add_edge_graph(&g1,"114","409",2.20983900578,0);
    add_edge_graph(&g1,"114","890",0.305543499321,0);
    add_edge_graph(&g1,"114","235",4.90737905104,0);
    add_edge_graph(&g1,"114","291",0.871705380964,0);
    add_edge_graph(&g1,"114","121",4.2557542913,0);
    add_edge_graph(&g1,"115","170",2.09329750129,0);
    add_edge_graph(&g1,"115","596",5.27606844395,0);
    add_edge_graph(&g1,"116","573",2.17657446663,0);
    add_edge_graph(&g1,"116","389",8.26930410861,0);
    add_edge_graph(&g1,"118","874",0.0612537046259,0);
    add_edge_graph(&g1,"119","139",3.82518924825,0);
    add_edge_graph(&g1,"119","498",2.3300061406,0);
    add_edge_graph(&g1,"119","596",2.72244805395,0);
    add_edge_graph(&g1,"119","888",4.07002301498,0);
    add_edge_graph(&g1,"119","253",8.36729444071,0);
    add_edge_graph(&g1,"120","129",9.52778566372,0);
    add_edge_graph(&g1,"120","133",4.61709970547,0);
    add_edge_graph(&g1,"120","394",3.19206182458,0);
    add_edge_graph(&g1,"120","139",1.67066149149,0);
    add_edge_graph(&g1,"120","396",3.56624204808,0);
    add_edge_graph(&g1,"120","399",2.44985745006,0);
    add_edge_graph(&g1,"120","272",2.00114633076,0);
    add_edge_graph(&g1,"120","786",0.301614179318,0);
    add_edge_graph(&g1,"120","662",1.49189732578,0);
    add_edge_graph(&g1,"120","151",4.32386555216,0);
    add_edge_graph(&g1,"120","152",6.32573823644,0);
    add_edge_graph(&g1,"120","282",7.99157684153,0);
    add_edge_graph(&g1,"120","286",4.12230010164,0);
    add_edge_graph(&g1,"120","800",2.66452580864,0);
    add_edge_graph(&g1,"120","678",9.43008638617,0);
    add_edge_graph(&g1,"120","170",3.13150096846,0);
    add_edge_graph(&g1,"120","890",7.33962041672,0);
    add_edge_graph(&g1,"120","812",9.96730081713,0);
    add_edge_graph(&g1,"120","557",4.20778485353,0);
    add_edge_graph(&g1,"120","155",7.60207194006,0);
    add_edge_graph(&g1,"120","862",8.19678272287,0);
    add_edge_graph(&g1,"120","950",1.29610835616,0);
    add_edge_graph(&g1,"120","439",2.44184727027,0);
    add_edge_graph(&g1,"120","824",6.33779959683,0);
    add_edge_graph(&g1,"120","953",4.83139630232,0);
    add_edge_graph(&g1,"120","317",6.00658508642,0);
    add_edge_graph(&g1,"120","193",5.40389894084,0);
    add_edge_graph(&g1,"120","578",9.15312379973,0);
    add_edge_graph(&g1,"120","779",0.226300609736,0);
    add_edge_graph(&g1,"120","970",2.99092806112,0);
    add_edge_graph(&g1,"120","205",5.23413424442,0);
    add_edge_graph(&g1,"120","288",0.884990777015,0);
    add_edge_graph(&g1,"120","210",4.69847267521,0);
    add_edge_graph(&g1,"120","596",6.13515499943,0);
    add_edge_graph(&g1,"120","473",0.640347946674,0);
    add_edge_graph(&g1,"120","493",6.12107358743,0);
    add_edge_graph(&g1,"120","348",5.14316086603,0);
    add_edge_graph(&g1,"120","174",8.75447630456,0);
    add_edge_graph(&g1,"120","350",2.12347522756,0);
    add_edge_graph(&g1,"120","353",9.84979924321,0);
    add_edge_graph(&g1,"120","357",4.32880267181,0);
    add_edge_graph(&g1,"120","872",6.4431175972,0);
    add_edge_graph(&g1,"120","874",2.35571237544,0);
    add_edge_graph(&g1,"120","876",2.95630414415,0);
    add_edge_graph(&g1,"120","402",4.83433293227,0);
    add_edge_graph(&g1,"120","366",9.157908723,0);
    add_edge_graph(&g1,"120","744",7.98036442119,0);
    add_edge_graph(&g1,"120","244",4.95638115786,0);
    add_edge_graph(&g1,"120","628",0.37660888153,0);
    add_edge_graph(&g1,"120","759",8.94133015907,0);
    add_edge_graph(&g1,"120","888",8.90942459586,0);
    add_edge_graph(&g1,"120","506",5.28158656494,0);
    add_edge_graph(&g1,"120","511",0.373062732912,0);
    add_edge_graph(&g1,"121","139",4.6698881716,0);
    add_edge_graph(&g1,"121","780",8.89805669786,0);
    add_edge_graph(&g1,"121","685",6.45175656575,0);
    add_edge_graph(&g1,"121","686",7.21121389969,0);
    add_edge_graph(&g1,"121","528",3.31512024207,0);
    add_edge_graph(&g1,"121","209",7.56400419094,0);
    add_edge_graph(&g1,"121","193",0.373502978806,0);
    add_edge_graph(&g1,"121","531",4.57847620234,0);
    add_edge_graph(&g1,"121","999",8.11837352048,0);
    add_edge_graph(&g1,"121","888",0.519377629143,0);
    add_edge_graph(&g1,"121","202",3.37876598591,0);
    add_edge_graph(&g1,"121","154",6.42369101384,0);
    add_edge_graph(&g1,"121","890",9.76478112785,0);
    add_edge_graph(&g1,"122","890",1.15134291566,0);
    add_edge_graph(&g1,"123","888",0.721097213822,0);
    add_edge_graph(&g1,"123","129",9.68045982157,0);
    add_edge_graph(&g1,"123","890",6.12281524852,0);
    add_edge_graph(&g1,"123","597",8.54714465805,0);
    add_edge_graph(&g1,"124","784",4.49020636614,0);
    add_edge_graph(&g1,"125","209",9.13414948674,0);
    add_edge_graph(&g1,"126","523",7.21088325143,0);
    add_edge_graph(&g1,"127","797",7.30869925829,0);
    add_edge_graph(&g1,"128","786",7.55618537249,0);
    add_edge_graph(&g1,"128","596",5.96791034762,0);
    add_edge_graph(&g1,"129","514",0.391724511654,0);
    add_edge_graph(&g1,"129","525",3.26010523648,0);
    add_edge_graph(&g1,"129","531",8.58635989035,0);
    add_edge_graph(&g1,"129","535",8.86105545478,0);
    add_edge_graph(&g1,"129","557",2.53982018465,0);
    add_edge_graph(&g1,"129","569",4.02882510995,0);
    add_edge_graph(&g1,"129","577",4.92878769502,0);
    add_edge_graph(&g1,"129","596",8.22178038565,0);
    add_edge_graph(&g1,"129","527",4.53072718088,0);
    add_edge_graph(&g1,"129","607",4.54913938601,0);
    add_edge_graph(&g1,"129","625",6.43278969268,0);
    add_edge_graph(&g1,"129","686",1.35396195246,0);
    add_edge_graph(&g1,"129","649",1.23406987714,0);
    add_edge_graph(&g1,"129","139",1.88467642237,0);
    add_edge_graph(&g1,"129","141",9.63934225964,0);
    add_edge_graph(&g1,"129","655",8.96734037522,0);
    add_edge_graph(&g1,"129","657",1.82345776723,0);
    add_edge_graph(&g1,"129","660",2.15978287504,0);
    add_edge_graph(&g1,"129","151",9.65687426026,0);
    add_edge_graph(&g1,"129","152",3.95965803038,0);
    add_edge_graph(&g1,"129","665",2.20037246513,0);
    add_edge_graph(&g1,"129","154",0.790629889549,0);
    add_edge_graph(&g1,"129","879",3.52628511603,0);
    add_edge_graph(&g1,"129","669",4.24882673949,0);
    add_edge_graph(&g1,"129","161",2.26197686983,0);
    add_edge_graph(&g1,"129","675",2.57572971243,0);
    add_edge_graph(&g1,"129","167",9.484156356,0);
    add_edge_graph(&g1,"129","170",8.6653000771,0);
    add_edge_graph(&g1,"129","171",3.58927832008,0);
    add_edge_graph(&g1,"129","174",5.06167500616,0);
    add_edge_graph(&g1,"129","690",1.17102865611,0);
    add_edge_graph(&g1,"129","691",9.46611399351,0);
    add_edge_graph(&g1,"129","698",5.60403791305,0);
    add_edge_graph(&g1,"129","193",9.87757940419,0);
    add_edge_graph(&g1,"129","708",7.31212354745,0);
    add_edge_graph(&g1,"129","709",4.96269019,0);
    add_edge_graph(&g1,"129","202",6.85708697472,0);
    add_edge_graph(&g1,"129","205",5.93680831704,0);
    add_edge_graph(&g1,"129","207",9.36934561813,0);
    add_edge_graph(&g1,"129","547",0.867242286046,0);
    add_edge_graph(&g1,"129","890",2.92063691548,0);
    add_edge_graph(&g1,"129","738",6.3459190273,0);
    add_edge_graph(&g1,"129","230",9.73771051729,0);
    add_edge_graph(&g1,"129","745",9.4781164256,0);
    add_edge_graph(&g1,"129","749",1.66011518778,0);
    add_edge_graph(&g1,"129","211",1.58124094287,0);
    add_edge_graph(&g1,"129","244",2.35269668946,0);
    add_edge_graph(&g1,"129","758",5.87779610437,0);
    add_edge_graph(&g1,"129","761",8.72301997905,0);
    add_edge_graph(&g1,"129","251",7.74570042219,0);
    add_edge_graph(&g1,"129","554",8.28554744782,0);
    add_edge_graph(&g1,"129","258",1.09331994461,0);
    add_edge_graph(&g1,"129","774",6.18193909673,0);
    add_edge_graph(&g1,"129","780",8.76991867679,0);
    add_edge_graph(&g1,"129","271",4.35929875047,0);
    add_edge_graph(&g1,"129","786",3.90960824574,0);
    add_edge_graph(&g1,"129","787",5.33657649217,0);
    add_edge_graph(&g1,"129","788",0.238713524678,0);
    add_edge_graph(&g1,"129","277",3.35859979618,0);
    add_edge_graph(&g1,"129","795",0.587235483904,0);
    add_edge_graph(&g1,"129","287",5.9952829132,0);
    add_edge_graph(&g1,"129","288",5.46296444683,0);
    add_edge_graph(&g1,"129","289",2.50060589344,0);
    add_edge_graph(&g1,"129","295",8.55492847807,0);
    add_edge_graph(&g1,"129","810",2.09104924405,0);
    add_edge_graph(&g1,"129","304",4.76587323043,0);
    add_edge_graph(&g1,"129","818",5.38365639086,0);
    add_edge_graph(&g1,"129","824",3.4506484531,0);
    add_edge_graph(&g1,"129","828",3.77505189864,0);
    add_edge_graph(&g1,"129","832",4.72922368627,0);
    add_edge_graph(&g1,"129","322",2.93071605575,0);
    add_edge_graph(&g1,"129","838",0.218349942867,0);
    add_edge_graph(&g1,"129","840",7.32566218962,0);
    add_edge_graph(&g1,"129","845",3.7293249528,0);
    add_edge_graph(&g1,"129","339",5.93066169596,0);
    add_edge_graph(&g1,"129","858",7.38225076577,0);
    add_edge_graph(&g1,"129","348",1.60077299574,0);
    add_edge_graph(&g1,"129","349",4.58023283229,0);
    add_edge_graph(&g1,"129","352",6.30403718208,0);
    add_edge_graph(&g1,"129","874",4.06386376726,0);
    add_edge_graph(&g1,"129","876",3.39220755318,0);
    add_edge_graph(&g1,"129","366",9.43411509659,0);
    add_edge_graph(&g1,"129","367",9.23202253979,0);
    add_edge_graph(&g1,"129","372",2.8631228108,0);
    add_edge_graph(&g1,"129","233",2.14625866105,0);
    add_edge_graph(&g1,"129","888",6.30593305582,0);
    add_edge_graph(&g1,"129","378",4.69721390105,0);
    add_edge_graph(&g1,"129","381",0.4359383335,0);
    add_edge_graph(&g1,"129","320",6.58963452993,0);
    add_edge_graph(&g1,"129","386",9.21364453729,0);
    add_edge_graph(&g1,"129","899",0.0157871329103,0);
    add_edge_graph(&g1,"129","389",7.03712063609,0);
    add_edge_graph(&g1,"129","396",4.97213177673,0);
    add_edge_graph(&g1,"129","398",7.9463485722,0);
    add_edge_graph(&g1,"129","914",4.27502834438,0);
    add_edge_graph(&g1,"129","403",2.06062865498,0);
    add_edge_graph(&g1,"129","404",4.22830217033,0);
    add_edge_graph(&g1,"129","414",8.63820723236,0);
    add_edge_graph(&g1,"129","417",1.44018395017,0);
    add_edge_graph(&g1,"129","667",6.58309183385,0);
    add_edge_graph(&g1,"129","326",7.9744706836,0);
    add_edge_graph(&g1,"129","157",3.78955230871,0);
    add_edge_graph(&g1,"129","945",5.11925596393,0);
    add_edge_graph(&g1,"129","954",3.48418932353,0);
    add_edge_graph(&g1,"129","450",0.767598669536,0);
    add_edge_graph(&g1,"129","963",0.732141751105,0);
    add_edge_graph(&g1,"129","968",7.24880467032,0);
    add_edge_graph(&g1,"129","458",9.25749011488,0);
    add_edge_graph(&g1,"129","460",1.38423333109,0);
    add_edge_graph(&g1,"129","973",5.36639453454,0);
    add_edge_graph(&g1,"129","473",6.66589500588,0);
    add_edge_graph(&g1,"129","986",7.56316037329,0);
    add_edge_graph(&g1,"129","476",4.40755040028,0);
    add_edge_graph(&g1,"129","990",8.3494869011,0);
    add_edge_graph(&g1,"129","421",8.60048493489,0);
    add_edge_graph(&g1,"129","493",5.24865024237,0);
    add_edge_graph(&g1,"129","253",0.301237603873,0);
    add_edge_graph(&g1,"129","502",6.2609189268,0);
    add_edge_graph(&g1,"129","511",8.65209148561,0);
    add_edge_graph(&g1,"130","154",8.13793172843,0);
    add_edge_graph(&g1,"130","874",0.3204489577,0);
    add_edge_graph(&g1,"131","547",5.76090000198,0);
    add_edge_graph(&g1,"132","786",2.52282410253,0);
    add_edge_graph(&g1,"132","795",7.41929149853,0);
    add_edge_graph(&g1,"134","698",4.52509664273,0);
    add_edge_graph(&g1,"135","874",2.83183362805,0);
    add_edge_graph(&g1,"135","284",7.9610729081,0);
    add_edge_graph(&g1,"136","890",8.31609599232,0);
    add_edge_graph(&g1,"137","651",4.95384720992,0);
    add_edge_graph(&g1,"137","139",3.39752192779,0);
    add_edge_graph(&g1,"138","874",0.109144437702,0);
    add_edge_graph(&g1,"139","518",9.27386368742,0);
    add_edge_graph(&g1,"139","527",2.75182888797,0);
    add_edge_graph(&g1,"139","528",6.55796716957,0);
    add_edge_graph(&g1,"139","531",0.972705794859,0);
    add_edge_graph(&g1,"139","557",9.27128887173,0);
    add_edge_graph(&g1,"139","558",1.23228950368,0);
    add_edge_graph(&g1,"139","582",1.45310582118,0);
    add_edge_graph(&g1,"139","596",7.6911115266,0);
    add_edge_graph(&g1,"139","607",5.91101613146,0);
    add_edge_graph(&g1,"139","610",1.19216228718,0);
    add_edge_graph(&g1,"139","617",1.59118893833,0);
    add_edge_graph(&g1,"139","622",9.03241442348,0);
    add_edge_graph(&g1,"139","628",8.34391801345,0);
    add_edge_graph(&g1,"139","147",2.15318971769,0);
    add_edge_graph(&g1,"139","665",1.2632580211,0);
    add_edge_graph(&g1,"139","155",4.25554470459,0);
    add_edge_graph(&g1,"139","669",7.40614794021,0);
    add_edge_graph(&g1,"139","161",2.34999367909,0);
    add_edge_graph(&g1,"139","168",1.69175298985,0);
    add_edge_graph(&g1,"139","169",7.17563332431,0);
    add_edge_graph(&g1,"139","170",2.09483530182,0);
    add_edge_graph(&g1,"139","685",3.82508720223,0);
    add_edge_graph(&g1,"139","174",5.2147782685,0);
    add_edge_graph(&g1,"139","176",4.86484624162,0);
    add_edge_graph(&g1,"139","690",4.98214792455,0);
    add_edge_graph(&g1,"139","183",3.61747709848,0);
    add_edge_graph(&g1,"139","697",4.62492571888,0);
    add_edge_graph(&g1,"139","193",5.84087221245,0);
    add_edge_graph(&g1,"139","709",7.24071166574,0);
    add_edge_graph(&g1,"139","202",9.25066312031,0);
    add_edge_graph(&g1,"139","205",8.90391670479,0);
    add_edge_graph(&g1,"139","209",9.688639082,0);
    add_edge_graph(&g1,"139","547",1.21947787049,0);
    add_edge_graph(&g1,"139","219",1.5036609594,0);
    add_edge_graph(&g1,"139","222",4.48865320011,0);
    add_edge_graph(&g1,"139","741",6.91798629554,0);
    add_edge_graph(&g1,"139","744",1.31270064302,0);
    add_edge_graph(&g1,"139","747",4.66103887974,0);
    add_edge_graph(&g1,"139","748",9.7402856864,0);
    add_edge_graph(&g1,"139","428",3.68760351504,0);
    add_edge_graph(&g1,"139","239",2.47844747627,0);
    add_edge_graph(&g1,"139","211",7.08417965376,0);
    add_edge_graph(&g1,"139","759",6.98242489964,0);
    add_edge_graph(&g1,"139","761",6.59980149701,0);
    add_edge_graph(&g1,"139","764",1.50103621943,0);
    add_edge_graph(&g1,"139","258",3.51812314849,0);
    add_edge_graph(&g1,"139","776",1.86906632624,0);
    add_edge_graph(&g1,"139","266",7.79731252763,0);
    add_edge_graph(&g1,"139","272",9.4397472249,0);
    add_edge_graph(&g1,"139","273",5.16777420224,0);
    add_edge_graph(&g1,"139","786",3.88895577329,0);
    add_edge_graph(&g1,"139","791",7.0968499654,0);
    add_edge_graph(&g1,"139","795",1.95929379337,0);
    add_edge_graph(&g1,"139","284",1.84718222887,0);
    add_edge_graph(&g1,"139","289",7.7991524195,0);
    add_edge_graph(&g1,"139","294",0.00428751898754,0);
    add_edge_graph(&g1,"139","295",0.438797672503,0);
    add_edge_graph(&g1,"139","808",1.4441052173,0);
    add_edge_graph(&g1,"139","299",8.08302890865,0);
    add_edge_graph(&g1,"139","304",9.41025888215,0);
    add_edge_graph(&g1,"139","311",9.75025259573,0);
    add_edge_graph(&g1,"139","824",1.42631959676,0);
    add_edge_graph(&g1,"139","320",0.00515240861279,0);
    add_edge_graph(&g1,"139","323",7.56198813317,0);
    add_edge_graph(&g1,"139","838",2.88358040012,0);
    add_edge_graph(&g1,"139","327",0.390641218656,0);
    add_edge_graph(&g1,"139","332",9.27600959111,0);
    add_edge_graph(&g1,"139","337",2.16244241614,0);
    add_edge_graph(&g1,"139","851",9.56712258139,0);
    add_edge_graph(&g1,"139","340",7.13615844689,0);
    add_edge_graph(&g1,"139","345",0.389349305732,0);
    add_edge_graph(&g1,"139","354",6.05502018084,0);
    add_edge_graph(&g1,"139","874",0.7503008141,0);
    add_edge_graph(&g1,"139","373",9.70729540479,0);
    add_edge_graph(&g1,"139","888",8.84358982951,0);
    add_edge_graph(&g1,"139","890",8.57465251165,0);
    add_edge_graph(&g1,"139","385",0.938994172696,0);
    add_edge_graph(&g1,"139","898",9.00472674771,0);
    add_edge_graph(&g1,"139","389",9.72703626924,0);
    add_edge_graph(&g1,"139","397",1.11296453874,0);
    add_edge_graph(&g1,"139","913",8.67330148185,0);
    add_edge_graph(&g1,"139","402",2.17250586821,0);
    add_edge_graph(&g1,"139","922",4.4656112607,0);
    add_edge_graph(&g1,"139","414",4.43225756658,0);
    add_edge_graph(&g1,"139","931",9.26229682715,0);
    add_edge_graph(&g1,"139","940",5.24447369363,0);
    add_edge_graph(&g1,"139","432",2.40731608179,0);
    add_edge_graph(&g1,"139","433",7.84045359907,0);
    add_edge_graph(&g1,"139","950",1.0538887167,0);
    add_edge_graph(&g1,"139","448",7.74397227928,0);
    add_edge_graph(&g1,"139","961",5.26823837831,0);
    add_edge_graph(&g1,"139","450",6.99689287124,0);
    add_edge_graph(&g1,"139","453",5.91234045361,0);
    add_edge_graph(&g1,"139","969",4.66982375156,0);
    add_edge_graph(&g1,"139","458",7.71170027125,0);
    add_edge_graph(&g1,"139","459",3.07603960381,0);
    add_edge_graph(&g1,"139","471",4.75430316007,0);
    add_edge_graph(&g1,"139","473",3.26438199808,0);
    add_edge_graph(&g1,"139","476",2.84404895678,0);
    add_edge_graph(&g1,"139","995",5.08348636792,0);
    add_edge_graph(&g1,"139","486",9.00466485042,0);
    add_edge_graph(&g1,"139","593",1.13608920735,0);
    add_edge_graph(&g1,"139","493",7.02489321473,0);
    add_edge_graph(&g1,"139","496",0.389388126865,0);
    add_edge_graph(&g1,"139","502",1.32993264492,0);
    add_edge_graph(&g1,"139","511",7.36600949231,0);
    add_edge_graph(&g1,"140","376",8.24619659684,0);
    add_edge_graph(&g1,"141","888",1.95504940715,0);
    add_edge_graph(&g1,"141","554",7.95852021405,0);
    add_edge_graph(&g1,"141","547",4.54884768709,0);
    add_edge_graph(&g1,"142","890",8.81311225936,0);
    add_edge_graph(&g1,"143","890",9.07993315432,0);
    add_edge_graph(&g1,"143","502",1.21278052749,0);
    add_edge_graph(&g1,"143","167",6.27992594366,0);
    add_edge_graph(&g1,"145","170",8.0880499207,0);
    add_edge_graph(&g1,"146","152",1.9178531499,0);
    add_edge_graph(&g1,"146","170",2.91187381299,0);
    add_edge_graph(&g1,"146","890",3.94538125314,0);
    add_edge_graph(&g1,"147","211",7.00649204926,0);
    add_edge_graph(&g1,"148","888",8.42844939662,0);
    add_edge_graph(&g1,"148","658",1.2287099721,0);
    add_edge_graph(&g1,"149","888",9.28904967354,0);
    add_edge_graph(&g1,"149","557",4.96674278475,0);
    add_edge_graph(&g1,"150","650",9.58174812631,0);
    add_edge_graph(&g1,"150","874",6.13854603219,0);
    add_edge_graph(&g1,"150","170",9.7174715302,0);
    add_edge_graph(&g1,"150","798",0.970838578885,0);
    add_edge_graph(&g1,"151","531",8.67792165961,0);
    add_edge_graph(&g1,"151","535",4.80073872835,0);
    add_edge_graph(&g1,"151","795",8.59714356655,0);
    add_edge_graph(&g1,"151","163",8.00629651657,0);
    add_edge_graph(&g1,"151","950",5.05795679416,0);
    add_edge_graph(&g1,"151","446",1.87085834327,0);
    add_edge_graph(&g1,"151","959",4.39232356204,0);
    add_edge_graph(&g1,"151","963",1.57227843605,0);
    add_edge_graph(&g1,"151","198",6.47065983895,0);
    add_edge_graph(&g1,"151","333",9.37680333362,0);
    add_edge_graph(&g1,"151","210",6.54195488349,0);
    add_edge_graph(&g1,"151","596",5.3258876282,0);
    add_edge_graph(&g1,"151","855",6.45270906276,0);
    add_edge_graph(&g1,"151","874",5.93086031549,0);
    add_edge_graph(&g1,"151","888",9.72166430674,0);
    add_edge_graph(&g1,"151","761",2.97018118423,0);
    add_edge_graph(&g1,"151","890",4.54775006302,0);
    add_edge_graph(&g1,"151","253",8.22712828469,0);
    add_edge_graph(&g1,"151","511",3.69119155342,0);
    add_edge_graph(&g1,"152","450",3.85662368821,0);
    add_edge_graph(&g1,"152","389",3.23793422546,0);
    add_edge_graph(&g1,"152","457",1.51356257093,0);
    add_edge_graph(&g1,"152","874",1.05244660891,0);
    add_edge_graph(&g1,"152","685",2.62114875684,0);
    add_edge_graph(&g1,"152","334",3.18122387665,0);
    add_edge_graph(&g1,"152","170",6.5216817634,0);
    add_edge_graph(&g1,"152","720",9.07746976327,0);
    add_edge_graph(&g1,"152","209",0.815993102172,0);
    add_edge_graph(&g1,"152","596",5.08233686292,0);
    add_edge_graph(&g1,"152","547",1.56266673221,0);
    add_edge_graph(&g1,"152","739",9.30973103058,0);
    add_edge_graph(&g1,"152","202",7.06547858637,0);
    add_edge_graph(&g1,"152","950",4.24405885229,0);
    add_edge_graph(&g1,"152","476",0.0389708256872,0);
    add_edge_graph(&g1,"152","573",0.86973236898,0);
    add_edge_graph(&g1,"153","890",8.85324040487,0);
    add_edge_graph(&g1,"154","897",4.36883605465,0);
    add_edge_graph(&g1,"154","904",1.65800466634,0);
    add_edge_graph(&g1,"154","527",0.227942837759,0);
    add_edge_graph(&g1,"154","531",4.89426566657,0);
    add_edge_graph(&g1,"154","176",7.48300662581,0);
    add_edge_graph(&g1,"154","819",8.63880780855,0);
    add_edge_graph(&g1,"154","845",1.2047411635,0);
    add_edge_graph(&g1,"154","209",5.20978926667,0);
    add_edge_graph(&g1,"154","855",0.725482528913,0);
    add_edge_graph(&g1,"154","473",7.18411038391,0);
    add_edge_graph(&g1,"154","490",3.43242754445,0);
    add_edge_graph(&g1,"154","628",7.38449720455,0);
    add_edge_graph(&g1,"154","888",8.84141980442,0);
    add_edge_graph(&g1,"154","890",8.72049735815,0);
    add_edge_graph(&g1,"154","508",7.14605618272,0);
    add_edge_graph(&g1,"155","295",7.73931529361,0);
    add_edge_graph(&g1,"155","808",4.26824515327,0);
    add_edge_graph(&g1,"155","604",8.15334007684,0);
    add_edge_graph(&g1,"155","173",9.45088274129,0);
    add_edge_graph(&g1,"155","174",9.34014303887,0);
    add_edge_graph(&g1,"155","528",2.65158024048,0);
    add_edge_graph(&g1,"155","888",9.67963915145,0);
    add_edge_graph(&g1,"155","852",7.01110334966,0);
    add_edge_graph(&g1,"155","814",2.41362384001,0);
    add_edge_graph(&g1,"155","761",6.03914100316,0);
    add_edge_graph(&g1,"155","596",1.02208685151,0);
    add_edge_graph(&g1,"155","186",4.94556976821,0);
    add_edge_graph(&g1,"155","795",3.63945857642,0);
    add_edge_graph(&g1,"155","508",5.10381746954,0);
    add_edge_graph(&g1,"155","170",8.64566714334,0);
    add_edge_graph(&g1,"156","429",4.48332225768,0);
    add_edge_graph(&g1,"157","389",1.91188714079,0);
    add_edge_graph(&g1,"157","295",5.14195614437,0);
    add_edge_graph(&g1,"158","874",8.70951543031,0);
    add_edge_graph(&g1,"159","888",7.28591598048,0);
    add_edge_graph(&g1,"160","193",4.50624455062,0);
    add_edge_graph(&g1,"160","810",9.40931506273,0);
    add_edge_graph(&g1,"161","182",7.93597175547,0);
    add_edge_graph(&g1,"161","890",4.40808194116,0);
    add_edge_graph(&g1,"161","442",6.24858866579,0);
    add_edge_graph(&g1,"162","332",6.1973548334,0);
    add_edge_graph(&g1,"162","527",8.21345844341,0);
    add_edge_graph(&g1,"163","535",5.75919709495,0);
    add_edge_graph(&g1,"164","890",5.26965146936,0);
    add_edge_graph(&g1,"164","174",1.66572368542,0);
    add_edge_graph(&g1,"165","874",5.27473743072,0);
    add_edge_graph(&g1,"165","596",1.70177818296,0);
    add_edge_graph(&g1,"166","795",5.5418421143,0);
    add_edge_graph(&g1,"168","874",8.3533563296,0);
    add_edge_graph(&g1,"168","527",3.75137134946,0);
    add_edge_graph(&g1,"168","596",4.06663241739,0);
    add_edge_graph(&g1,"168","821",4.9021803653,0);
    add_edge_graph(&g1,"168","918",6.69241126446,0);
    add_edge_graph(&g1,"168","759",1.40877583554,0);
    add_edge_graph(&g1,"168","888",0.16685524234,0);
    add_edge_graph(&g1,"168","810",6.5478828724,0);
    add_edge_graph(&g1,"169","484",1.57194247014,0);
    add_edge_graph(&g1,"169","709",6.22247914957,0);
    add_edge_graph(&g1,"169","874",8.11493197547,0);
    add_edge_graph(&g1,"169","527",5.90726349948,0);
    add_edge_graph(&g1,"169","792",1.0101958886,0);
    add_edge_graph(&g1,"169","182",9.70554079172,0);
    add_edge_graph(&g1,"169","888",0.84508804786,0);
    add_edge_graph(&g1,"169","890",3.364731359,0);
    add_edge_graph(&g1,"170","527",7.63945155296,0);
    add_edge_graph(&g1,"170","529",2.29588129182,0);
    add_edge_graph(&g1,"170","549",3.39606565166,0);
    add_edge_graph(&g1,"170","557",1.67051590309,0);
    add_edge_graph(&g1,"170","587",9.27096834844,0);
    add_edge_graph(&g1,"170","590",0.717380732976,0);
    add_edge_graph(&g1,"170","596",3.17097687285,0);
    add_edge_graph(&g1,"170","610",5.56261133963,0);
    add_edge_graph(&g1,"170","615",5.53061560422,0);
    add_edge_graph(&g1,"170","686",2.71841010815,0);
    add_edge_graph(&g1,"170","646",2.56240814375,0);
    add_edge_graph(&g1,"170","193",1.54283911908,0);
    add_edge_graph(&g1,"170","660",3.01117618197,0);
    add_edge_graph(&g1,"170","674",4.12674188566,0);
    add_edge_graph(&g1,"170","795",9.21832892555,0);
    add_edge_graph(&g1,"170","677",6.42020865973,0);
    add_edge_graph(&g1,"170","174",2.48253822108,0);
    add_edge_graph(&g1,"170","689",3.04035374083,0);
    add_edge_graph(&g1,"170","697",0.711436906635,0);
    add_edge_graph(&g1,"170","705",8.28466328791,0);
    add_edge_graph(&g1,"170","707",7.29867249485,0);
    add_edge_graph(&g1,"170","198",7.59289071938,0);
    add_edge_graph(&g1,"170","202",7.2067640576,0);
    add_edge_graph(&g1,"170","205",4.96685973377,0);
    add_edge_graph(&g1,"170","222",8.90843751336,0);
    add_edge_graph(&g1,"170","230",2.51464296058,0);
    add_edge_graph(&g1,"170","743",2.59421677945,0);
    add_edge_graph(&g1,"170","237",8.09934035128,0);
    add_edge_graph(&g1,"170","251",5.26727597856,0);
    add_edge_graph(&g1,"170","764",8.16735640544,0);
    add_edge_graph(&g1,"170","771",9.83278823117,0);
    add_edge_graph(&g1,"170","271",7.0539334594,0);
    add_edge_graph(&g1,"170","786",5.24354708996,0);
    add_edge_graph(&g1,"170","283",5.93614759405,0);
    add_edge_graph(&g1,"170","797",8.17014158974,0);
    add_edge_graph(&g1,"170","799",9.41818802076,0);
    add_edge_graph(&g1,"170","289",6.56263219475,0);
    add_edge_graph(&g1,"170","295",1.07580003982,0);
    add_edge_graph(&g1,"170","810",3.33581935078,0);
    add_edge_graph(&g1,"170","903",8.66001977723,0);
    add_edge_graph(&g1,"170","824",9.39232293768,0);
    add_edge_graph(&g1,"170","324",5.94188959911,0);
    add_edge_graph(&g1,"170","332",5.72049283022,0);
    add_edge_graph(&g1,"170","850",5.70991844533,0);
    add_edge_graph(&g1,"170","851",5.49727141433,0);
    add_edge_graph(&g1,"170","346",4.69922624187,0);
    add_edge_graph(&g1,"170","859",7.15382218131,0);
    add_edge_graph(&g1,"170","349",9.28729522477,0);
    add_edge_graph(&g1,"170","866",6.82961295699,0);
    add_edge_graph(&g1,"170","870",7.12026465649,0);
    add_edge_graph(&g1,"170","874",3.78698627903,0);
    add_edge_graph(&g1,"170","876",0.466844714467,0);
    add_edge_graph(&g1,"170","888",9.82779377378,0);
    add_edge_graph(&g1,"170","890",0.283619265733,0);
    add_edge_graph(&g1,"170","381",2.40427592183,0);
    add_edge_graph(&g1,"170","896",7.51627981758,0);
    add_edge_graph(&g1,"170","385",3.3622251422,0);
    add_edge_graph(&g1,"170","899",4.44534924724,0);
    add_edge_graph(&g1,"170","900",6.03558597308,0);
    add_edge_graph(&g1,"170","391",1.56051082789,0);
    add_edge_graph(&g1,"170","404",7.32751274176,0);
    add_edge_graph(&g1,"170","921",1.65344368212,0);
    add_edge_graph(&g1,"170","927",4.41829163744,0);
    add_edge_graph(&g1,"170","423",2.97881614734,0);
    add_edge_graph(&g1,"170","937",9.69412252433,0);
    add_edge_graph(&g1,"170","427",7.41389672027,0);
    add_edge_graph(&g1,"170","986",3.06887475839,0);
    add_edge_graph(&g1,"170","951",9.18911189478,0);
    add_edge_graph(&g1,"170","440",2.01472654861,0);
    add_edge_graph(&g1,"170","957",5.26483966721,0);
    add_edge_graph(&g1,"170","960",0.431100062774,0);
    add_edge_graph(&g1,"170","450",1.14294643338,0);
    add_edge_graph(&g1,"170","976",9.78070974449,0);
    add_edge_graph(&g1,"170","675",3.53350591617,0);
    add_edge_graph(&g1,"170","981",5.10153983996,0);
    add_edge_graph(&g1,"170","473",1.4906672819,0);
    add_edge_graph(&g1,"170","474",6.14199604931,0);
    add_edge_graph(&g1,"170","478",0.985483890925,0);
    add_edge_graph(&g1,"170","480",9.95803734814,0);
    add_edge_graph(&g1,"170","997",1.01415791587,0);
    add_edge_graph(&g1,"170","339",2.89600314979,0);
    add_edge_graph(&g1,"170","511",3.64876166623,0);
    add_edge_graph(&g1,"172","890",1.14794307991,0);
    add_edge_graph(&g1,"172","182",6.00857848897,0);
    add_edge_graph(&g1,"173","524",7.28074844239,0);
    add_edge_graph(&g1,"174","898",1.41450590922,0);
    add_edge_graph(&g1,"174","904",3.22653786918,0);
    add_edge_graph(&g1,"174","783",5.72933364716,0);
    add_edge_graph(&g1,"174","912",1.00008054826,0);
    add_edge_graph(&g1,"174","786",4.80536710908,0);
    add_edge_graph(&g1,"174","531",7.96402570347,0);
    add_edge_graph(&g1,"174","660",2.88826902273,0);
    add_edge_graph(&g1,"174","918",2.13854355663,0);
    add_edge_graph(&g1,"174","409",2.55276341661,0);
    add_edge_graph(&g1,"174","794",4.28895375674,0);
    add_edge_graph(&g1,"174","304",6.91717018539,0);
    add_edge_graph(&g1,"174","549",8.90790135703,0);
    add_edge_graph(&g1,"174","295",4.31931830459,0);
    add_edge_graph(&g1,"174","936",6.58459735792,0);
    add_edge_graph(&g1,"174","940",2.78900918813,0);
    add_edge_graph(&g1,"174","557",8.70535534391,0);
    add_edge_graph(&g1,"174","302",5.72158295104,0);
    add_edge_graph(&g1,"174","303",1.19497834413,0);
    add_edge_graph(&g1,"174","888",7.41535587207,0);
    add_edge_graph(&g1,"174","178",9.88327225017,0);
    add_edge_graph(&g1,"174","824",0.0791748858481,0);
    add_edge_graph(&g1,"174","803",9.98116448048,0);
    add_edge_graph(&g1,"174","575",8.66402664463,0);
    add_edge_graph(&g1,"174","962",4.08659054612,0);
    add_edge_graph(&g1,"174","963",0.67811226009,0);
    add_edge_graph(&g1,"174","455",7.90413638029,0);
    add_edge_graph(&g1,"174","968",5.41407607559,0);
    add_edge_graph(&g1,"174","205",6.07751468862,0);
    add_edge_graph(&g1,"174","850",7.94104375665,0);
    add_edge_graph(&g1,"174","211",3.94902567854,0);
    add_edge_graph(&g1,"174","596",0.672901428696,0);
    add_edge_graph(&g1,"174","470",0.26900214259,0);
    add_edge_graph(&g1,"174","217",1.17328747709,0);
    add_edge_graph(&g1,"174","207",4.20703085565,0);
    add_edge_graph(&g1,"174","356",5.20456851961,0);
    add_edge_graph(&g1,"174","230",1.18995371074,0);
    add_edge_graph(&g1,"174","874",9.18775663052,0);
    add_edge_graph(&g1,"174","890",9.57026129464,0);
    add_edge_graph(&g1,"175","256",5.86309287765,0);
    add_edge_graph(&g1,"175","193",2.60247964045,0);
    add_edge_graph(&g1,"175","323",4.99698478535,0);
    add_edge_graph(&g1,"175","202",4.51868733673,0);
    add_edge_graph(&g1,"175","527",7.23166683435,0);
    add_edge_graph(&g1,"175","786",7.19304148769,0);
    add_edge_graph(&g1,"175","890",0.9178423418,0);
    add_edge_graph(&g1,"176","771",6.0799223029,0);
    add_edge_graph(&g1,"176","332",8.24783880286,0);
    add_edge_graph(&g1,"176","209",0.517810129027,0);
    add_edge_graph(&g1,"176","596",2.3748554302,0);
    add_edge_graph(&g1,"176","182",7.32339582152,0);
    add_edge_graph(&g1,"176","473",0.967066956996,0);
    add_edge_graph(&g1,"176","697",8.90447433026,0);
    add_edge_graph(&g1,"176","476",4.08302797153,0);
    add_edge_graph(&g1,"176","890",7.78643936459,0);
    add_edge_graph(&g1,"177","858",0.410199958058,0);
    add_edge_graph(&g1,"178","202",2.72926052945,0);
    add_edge_graph(&g1,"178","855",0.308490569566,0);
    add_edge_graph(&g1,"178","596",7.58604614938,0);
    add_edge_graph(&g1,"179","780",2.01866419957,0);
    add_edge_graph(&g1,"179","303",8.05712732643,0);
    add_edge_graph(&g1,"180","253",3.59424893579,0);
    add_edge_graph(&g1,"180","894",6.67930125113,0);
    add_edge_graph(&g1,"181","888",8.41366858212,0);
    add_edge_graph(&g1,"181","939",4.49467983,0);
    add_edge_graph(&g1,"182","642",3.8503474511,0);
    add_edge_graph(&g1,"182","906",7.04493647023,0);
    add_edge_graph(&g1,"182","780",3.31766529518,0);
    add_edge_graph(&g1,"182","527",7.68314090768,0);
    add_edge_graph(&g1,"182","411",4.62008631326,0);
    add_edge_graph(&g1,"182","542",2.48794914919,0);
    add_edge_graph(&g1,"182","804",5.33983606523,0);
    add_edge_graph(&g1,"182","950",1.8991683941,0);
    add_edge_graph(&g1,"182","824",3.60624418326,0);
    add_edge_graph(&g1,"182","825",3.18440875823,0);
    add_edge_graph(&g1,"182","211",1.62067594324,0);
    add_edge_graph(&g1,"182","596",3.27922864526,0);
    add_edge_graph(&g1,"182","874",9.6700503087,0);
    add_edge_graph(&g1,"182","502",0.682554150964,0);
    add_edge_graph(&g1,"182","890",8.80551013982,0);
    add_edge_graph(&g1,"182","767",1.38926039464,0);
    add_edge_graph(&g1,"183","640",9.73241425298,0);
    add_edge_graph(&g1,"184","473",2.90459388056,0);
    add_edge_graph(&g1,"184","890",0.510561178606,0);
    add_edge_graph(&g1,"185","683",0.177775097778,0);
    add_edge_graph(&g1,"185","823",7.87786514475,0);
    add_edge_graph(&g1,"187","304",9.73337522936,0);
    add_edge_graph(&g1,"187","458",4.66850211193,0);
    add_edge_graph(&g1,"187","890",0.984006246114,0);
    add_edge_graph(&g1,"187","527",2.26521745055,0);
    add_edge_graph(&g1,"188","874",3.26609224515,0);
    add_edge_graph(&g1,"188","685",4.3347433671,0);
    add_edge_graph(&g1,"189","272",3.1269781741,0);
    add_edge_graph(&g1,"190","713",8.69590954935,0);
    add_edge_graph(&g1,"190","423",8.47791052921,0);
    add_edge_graph(&g1,"191","474",2.94598736981,0);
    add_edge_graph(&g1,"191","874",1.84749656579,0);
    add_edge_graph(&g1,"191","890",8.83715472521,0);
    add_edge_graph(&g1,"192","258",5.43222030138,0);
    add_edge_graph(&g1,"193","772",5.07190177132,0);
    add_edge_graph(&g1,"193","901",1.96131012488,0);
    add_edge_graph(&g1,"193","777",7.84271281886,0);
    add_edge_graph(&g1,"193","780",2.36154775816,0);
    add_edge_graph(&g1,"193","397",9.49517915997,0);
    add_edge_graph(&g1,"193","654",8.07759131192,0);
    add_edge_graph(&g1,"193","527",2.07964096086,0);
    add_edge_graph(&g1,"193","531",1.96414402664,0);
    add_edge_graph(&g1,"193","404",0.817132377838,0);
    add_edge_graph(&g1,"193","884",2.81902497716,0);
    add_edge_graph(&g1,"193","930",5.88190310196,0);
    add_edge_graph(&g1,"193","289",5.45321706796,0);
    add_edge_graph(&g1,"193","290",6.29728159591,0);
    add_edge_graph(&g1,"193","419",4.5176210545,0);
    add_edge_graph(&g1,"193","845",0.954181143503,0);
    add_edge_graph(&g1,"193","937",7.3219383421,0);
    add_edge_graph(&g1,"193","427",6.61423253065,0);
    add_edge_graph(&g1,"193","304",6.26300783804,0);
    add_edge_graph(&g1,"193","306",5.87648183385,0);
    add_edge_graph(&g1,"193","308",6.12946161051,0);
    add_edge_graph(&g1,"193","693",6.8022891449,0);
    add_edge_graph(&g1,"193","439",3.45126613957,0);
    add_edge_graph(&g1,"193","440",3.10801141653,0);
    add_edge_graph(&g1,"193","825",8.88978349539,0);
    add_edge_graph(&g1,"193","315",0.234139986954,0);
    add_edge_graph(&g1,"193","194",8.67734490088,0);
    add_edge_graph(&g1,"193","963",0.403351627006,0);
    add_edge_graph(&g1,"193","453",7.22995581379,0);
    add_edge_graph(&g1,"193","710",3.79035613527,0);
    add_edge_graph(&g1,"193","567",5.88637688513,0);
    add_edge_graph(&g1,"193","332",9.67409743383,0);
    add_edge_graph(&g1,"193","205",9.33145679193,0);
    add_edge_graph(&g1,"193","888",3.372864339,0);
    add_edge_graph(&g1,"193","596",8.9601629855,0);
    add_edge_graph(&g1,"193","470",2.27273503841,0);
    add_edge_graph(&g1,"193","855",9.10242278849,0);
    add_edge_graph(&g1,"193","557",5.70060420048,0);
    add_edge_graph(&g1,"193","916",8.90875419425,0);
    add_edge_graph(&g1,"193","209",6.90626716715,0);
    add_edge_graph(&g1,"193","874",3.34852965155,0);
    add_edge_graph(&g1,"193","752",2.38266578811,0);
    add_edge_graph(&g1,"193","627",2.10884631061,0);
    add_edge_graph(&g1,"193","244",6.53995253974,0);
    add_edge_graph(&g1,"193","890",5.37241908025,0);
    add_edge_graph(&g1,"193","709",2.16335972433,0);
    add_edge_graph(&g1,"193","253",2.24496776142,0);
    add_edge_graph(&g1,"195","874",8.65981775045,0);
    add_edge_graph(&g1,"195","837",2.28400508125,0);
    add_edge_graph(&g1,"196","473",5.73405585353,0);
    add_edge_graph(&g1,"196","474",3.62804269424,0);
    add_edge_graph(&g1,"197","888",3.63819934628,0);
    add_edge_graph(&g1,"197","874",0.281885076842,0);
    add_edge_graph(&g1,"198","429",7.03871037042,0);
    add_edge_graph(&g1,"198","376",3.57584952517,0);
    add_edge_graph(&g1,"198","402",0.754329304399,0);
    add_edge_graph(&g1,"198","531",8.87209282268,0);
    add_edge_graph(&g1,"198","596",8.58629139859,0);
    add_edge_graph(&g1,"198","888",8.76307554663,0);
    add_edge_graph(&g1,"198","890",6.91856789646,0);
    add_edge_graph(&g1,"198","874",5.15393956721,0);
    add_edge_graph(&g1,"199","685",4.35388533545,0);
    add_edge_graph(&g1,"199","557",3.71594286678,0);
    add_edge_graph(&g1,"200","295",6.27945969939,0);
    add_edge_graph(&g1,"201","874",6.76947953208,0);
    add_edge_graph(&g1,"201","786",6.8304598177,0);
    add_edge_graph(&g1,"202","517",3.86621283901,0);
    add_edge_graph(&g1,"202","391",7.47564750934,0);
    add_edge_graph(&g1,"202","909",0.878446639532,0);
    add_edge_graph(&g1,"202","786",3.92589003818,0);
    add_edge_graph(&g1,"202","531",9.23141760733,0);
    add_edge_graph(&g1,"202","303",8.69070564431,0);
    add_edge_graph(&g1,"202","293",9.33290657249,0);
    add_edge_graph(&g1,"202","433",8.40436531015,0);
    add_edge_graph(&g1,"202","941",6.90046389588,0);
    add_edge_graph(&g1,"202","945",0.708332006361,0);
    add_edge_graph(&g1,"202","950",9.79775856478,0);
    add_edge_graph(&g1,"202","833",3.78400482634,0);
    add_edge_graph(&g1,"202","954",8.73201144648,0);
    add_edge_graph(&g1,"202","443",9.88595155474,0);
    add_edge_graph(&g1,"202","574",7.22187834144,0);
    add_edge_graph(&g1,"202","705",0.737062642802,0);
    add_edge_graph(&g1,"202","835",5.37518432131,0);
    add_edge_graph(&g1,"202","795",8.53352972454,0);
    add_edge_graph(&g1,"202","459",2.22186427436,0);
    add_edge_graph(&g1,"202","332",9.68142333657,0);
    add_edge_graph(&g1,"202","205",5.13540418675,0);
    add_edge_graph(&g1,"202","211",5.99226411202,0);
    add_edge_graph(&g1,"202","596",1.43385631868,0);
    add_edge_graph(&g1,"202","470",5.87299041252,0);
    add_edge_graph(&g1,"202","600",5.29230233822,0);
    add_edge_graph(&g1,"202","602",6.0258426769,0);
    add_edge_graph(&g1,"202","527",7.67114689607,0);
    add_edge_graph(&g1,"202","557",9.50453956771,0);
    add_edge_graph(&g1,"202","800",1.99553533629,0);
    add_edge_graph(&g1,"202","356",8.56967551147,0);
    add_edge_graph(&g1,"202","744",2.29492243689,0);
    add_edge_graph(&g1,"202","489",1.64689424072,0);
    add_edge_graph(&g1,"202","874",4.17693406422,0);
    add_edge_graph(&g1,"202","295",9.83750596896,0);
    add_edge_graph(&g1,"202","366",2.22680790345,0);
    add_edge_graph(&g1,"202","467",2.58495089749,0);
    add_edge_graph(&g1,"202","547",3.18692368903,0);
    add_edge_graph(&g1,"202","888",1.09271995038,0);
    add_edge_graph(&g1,"202","890",7.37613951802,0);
    add_edge_graph(&g1,"202","351",0.537546577047,0);
    add_edge_graph(&g1,"203","840",0.281236680312,0);
    add_edge_graph(&g1,"203","874",6.74801402362,0);
    add_edge_graph(&g1,"203","685",4.90696702264,0);
    add_edge_graph(&g1,"203","881",2.75088764073,0);
    add_edge_graph(&g1,"203","638",1.23602162312,0);
    add_edge_graph(&g1,"204","353",5.4901530389,0);
    add_edge_graph(&g1,"204","729",1.47758724075,0);
    add_edge_graph(&g1,"204","557",6.45458335868,0);
    add_edge_graph(&g1,"204","799",7.12072313989,0);
    add_edge_graph(&g1,"205","384",1.2062753808,0);
    add_edge_graph(&g1,"205","528",1.52374876109,0);
    add_edge_graph(&g1,"205","786",6.9588778104,0);
    add_edge_graph(&g1,"205","531",2.91681621561,0);
    add_edge_graph(&g1,"205","404",0.94054906294,0);
    add_edge_graph(&g1,"205","857",4.12465464893,0);
    add_edge_graph(&g1,"205","818",5.11408451226,0);
    add_edge_graph(&g1,"205","439",6.50989860898,0);
    add_edge_graph(&g1,"205","571",2.01948020755,0);
    add_edge_graph(&g1,"205","573",7.01324673211,0);
    add_edge_graph(&g1,"205","458",1.07261633893,0);
    add_edge_graph(&g1,"205","547",8.11961992059,0);
    add_edge_graph(&g1,"205","596",2.15816547252,0);
    add_edge_graph(&g1,"205","473",1.16144592319,0);
    add_edge_graph(&g1,"205","349",5.52459899223,0);
    add_edge_graph(&g1,"205","230",2.46635045309,0);
    add_edge_graph(&g1,"205","615",3.77618938504,0);
    add_edge_graph(&g1,"205","874",0.478355893836,0);
    add_edge_graph(&g1,"205","359",3.10769774824,0);
    add_edge_graph(&g1,"205","890",7.51719628476,0);
    add_edge_graph(&g1,"205","366",4.36814632948,0);
    add_edge_graph(&g1,"205","957",8.98397913538,0);
    add_edge_graph(&g1,"205","881",1.54810400552,0);
    add_edge_graph(&g1,"205","498",5.11689603408,0);
    add_edge_graph(&g1,"205","888",3.70212626525,0);
    add_edge_graph(&g1,"205","762",0.531880790249,0);
    add_edge_graph(&g1,"205","511",0.406692361642,0);
    add_edge_graph(&g1,"207","786",0.772485710168,0);
    add_edge_graph(&g1,"208","991",4.40988786396,0);
    add_edge_graph(&g1,"209","899",4.10121202514,0);
    add_edge_graph(&g1,"209","527",1.84984971161,0);
    add_edge_graph(&g1,"209","786",3.1601909997,0);
    add_edge_graph(&g1,"209","531",6.33743176334,0);
    add_edge_graph(&g1,"209","795",0.0270563777056,0);
    add_edge_graph(&g1,"209","797",4.63172109987,0);
    add_edge_graph(&g1,"209","414",3.11489001804,0);
    add_edge_graph(&g1,"209","547",2.10109644236,0);
    add_edge_graph(&g1,"209","295",5.1718041111,0);
    add_edge_graph(&g1,"209","443",0.93603306376,0);
    add_edge_graph(&g1,"209","451",6.13178124453,0);
    add_edge_graph(&g1,"209","455",0.189493816805,0);
    add_edge_graph(&g1,"209","587",8.69570509213,0);
    add_edge_graph(&g1,"209","850",2.76145983077,0);
    add_edge_graph(&g1,"209","596",8.45116963412,0);
    add_edge_graph(&g1,"209","470",5.06374253172,0);
    add_edge_graph(&g1,"209","986",4.19835813353,0);
    add_edge_graph(&g1,"209","222",7.49239965576,0);
    add_edge_graph(&g1,"209","864",9.41886247894,0);
    add_edge_graph(&g1,"209","481",0.188506081003,0);
    add_edge_graph(&g1,"209","482",3.95075321485,0);
    add_edge_graph(&g1,"209","483",0.396180706515,0);
    add_edge_graph(&g1,"209","230",6.15673566226,0);
    add_edge_graph(&g1,"209","874",9.92029662055,0);
    add_edge_graph(&g1,"209","366",3.01451396982,0);
    add_edge_graph(&g1,"209","880",4.33155705071,0);
    add_edge_graph(&g1,"209","755",3.67275554461,0);
    add_edge_graph(&g1,"209","888",8.67569988124,0);
    add_edge_graph(&g1,"209","890",9.20541218924,0);
    add_edge_graph(&g1,"209","511",0.181798488771,0);
    add_edge_graph(&g1,"209","917",5.16920019216,0);
    add_edge_graph(&g1,"210","888",2.63268494022,0);
    add_edge_graph(&g1,"210","418",0.539587934264,0);
    add_edge_graph(&g1,"211","389",2.98801343539,0);
    add_edge_graph(&g1,"211","656",2.20595911603,0);
    add_edge_graph(&g1,"211","913",2.33470984101,0);
    add_edge_graph(&g1,"211","786",3.04255372947,0);
    add_edge_graph(&g1,"211","531",0.979702145612,0);
    add_edge_graph(&g1,"211","278",2.82188113607,0);
    add_edge_graph(&g1,"211","923",1.38607587121,0);
    add_edge_graph(&g1,"211","797",7.08631709366,0);
    add_edge_graph(&g1,"211","800",2.34959393858,0);
    add_edge_graph(&g1,"211","289",3.72547909678,0);
    add_edge_graph(&g1,"211","295",2.58124068753,0);
    add_edge_graph(&g1,"211","824",5.70456570698,0);
    add_edge_graph(&g1,"211","458",0.307866658054,0);
    add_edge_graph(&g1,"211","446",6.75729153719,0);
    add_edge_graph(&g1,"211","965",0.736536657764,0);
    add_edge_graph(&g1,"211","592",7.64812727031,0);
    add_edge_graph(&g1,"211","850",8.34610492392,0);
    add_edge_graph(&g1,"211","596",4.28361281304,0);
    add_edge_graph(&g1,"211","853",6.44709800639,0);
    add_edge_graph(&g1,"211","349",4.1635006519,0);
    add_edge_graph(&g1,"211","874",5.49529686337,0);
    add_edge_graph(&g1,"211","751",6.77725693463,0);
    add_edge_graph(&g1,"211","502",6.01001328298,0);
    add_edge_graph(&g1,"211","888",5.57293170743,0);
    add_edge_graph(&g1,"211","212",0.638300757217,0);
    add_edge_graph(&g1,"211","890",1.86992953983,0);
    add_edge_graph(&g1,"211","764",5.65672140312,0);
    add_edge_graph(&g1,"212","289",4.80929310306,0);
    add_edge_graph(&g1,"212","523",5.32820201353,0);
    add_edge_graph(&g1,"212","804",9.26772027421,0);
    add_edge_graph(&g1,"212","389",5.56425519743,0);
    add_edge_graph(&g1,"212","710",2.1109923347,0);
    add_edge_graph(&g1,"212","811",3.57123778816,0);
    add_edge_graph(&g1,"212","780",1.48843769222,0);
    add_edge_graph(&g1,"212","294",2.36841559162,0);
    add_edge_graph(&g1,"212","633",9.10596333035,0);
    add_edge_graph(&g1,"212","874",5.47762004925,0);
    add_edge_graph(&g1,"213","596",9.78335750027,0);
    add_edge_graph(&g1,"213","317",6.74937732311,0);
    add_edge_graph(&g1,"214","527",7.90268949679,0);
    add_edge_graph(&g1,"215","874",8.05523908914,0);
    add_edge_graph(&g1,"215","890",9.80708153291,0);
    add_edge_graph(&g1,"216","531",8.69760865166,0);
    add_edge_graph(&g1,"217","580",2.71780068514,0);
    add_edge_graph(&g1,"217","455",5.69336877497,0);
    add_edge_graph(&g1,"217","527",4.90592485714,0);
    add_edge_graph(&g1,"217","954",3.89554752088,0);
    add_edge_graph(&g1,"218","470",1.6256123754,0);
    add_edge_graph(&g1,"220","473",8.8925336307,0);
    add_edge_graph(&g1,"221","709",8.865170087,0);
    add_edge_graph(&g1,"222","395",6.67149486611,0);
    add_edge_graph(&g1,"222","502",9.52269749798,0);
    add_edge_graph(&g1,"222","521",8.02102566019,0);
    add_edge_graph(&g1,"222","876",8.14173465627,0);
    add_edge_graph(&g1,"222","874",9.88386599599,0);
    add_edge_graph(&g1,"222","227",0.618467048561,0);
    add_edge_graph(&g1,"222","596",5.14863443878,0);
    add_edge_graph(&g1,"222","950",5.94028771273,0);
    add_edge_graph(&g1,"222","888",8.82029356815,0);
    add_edge_graph(&g1,"222","890",4.01525037463,0);
    add_edge_graph(&g1,"222","389",3.22676451487,0);
    add_edge_graph(&g1,"223","874",6.33504583861,0);
    add_edge_graph(&g1,"223","596",4.71936600274,0);
    add_edge_graph(&g1,"224","881",2.94551554076,0);
    add_edge_graph(&g1,"225","916",4.25310916035,0);
    add_edge_graph(&g1,"225","557",8.46309173789,0);
    add_edge_graph(&g1,"226","596",6.39449957935,0);
    add_edge_graph(&g1,"226","527",0.430229784524,0);
    add_edge_graph(&g1,"228","456",0.12690179173,0);
    add_edge_graph(&g1,"228","511",9.65438906048,0);
    add_edge_graph(&g1,"228","295",8.2592870093,0);
    add_edge_graph(&g1,"229","940",4.39533469458,0);
    add_edge_graph(&g1,"229","786",6.27376423636,0);
    add_edge_graph(&g1,"229","238",7.52023298897,0);
    add_edge_graph(&g1,"229","498",2.9731109306,0);
    add_edge_graph(&g1,"229","890",0.750018605604,0);
    add_edge_graph(&g1,"230","485",5.54614243873,0);
    add_edge_graph(&g1,"230","490",0.749033197247,0);
    add_edge_graph(&g1,"230","396",7.86952982251,0);
    add_edge_graph(&g1,"230","527",0.967541674357,0);
    add_edge_graph(&g1,"230","596",7.76731534363,0);
    add_edge_graph(&g1,"230","782",0.522296426983,0);
    add_edge_graph(&g1,"230","473",1.77708815461,0);
    add_edge_graph(&g1,"230","890",4.4198237581,0);
    add_edge_graph(&g1,"230","874",6.11670809082,0);
    add_edge_graph(&g1,"230","511",1.38896046391,0);
    add_edge_graph(&g1,"231","832",2.54261936785,0);
    add_edge_graph(&g1,"231","257",4.04848508187,0);
    add_edge_graph(&g1,"231","389",0.80650716449,0);
    add_edge_graph(&g1,"231","870",3.47880375603,0);
    add_edge_graph(&g1,"231","295",7.63125148471,0);
    add_edge_graph(&g1,"231","557",9.92247508315,0);
    add_edge_graph(&g1,"231","850",1.18913737814,0);
    add_edge_graph(&g1,"231","596",8.7182947289,0);
    add_edge_graph(&g1,"231","888",7.05831204979,0);
    add_edge_graph(&g1,"231","628",7.83061030001,0);
    add_edge_graph(&g1,"232","888",9.98262758663,0);
    add_edge_graph(&g1,"232","382",5.43523437014,0);
    add_edge_graph(&g1,"234","874",5.64922298202,0);
    add_edge_graph(&g1,"234","890",6.55410065542,0);
    add_edge_graph(&g1,"234","866",9.39623020506,0);
    add_edge_graph(&g1,"234","295",0.146182576994,0);
    add_edge_graph(&g1,"236","473",1.30424751773,0);
    add_edge_graph(&g1,"236","717",7.52607251655,0);
    add_edge_graph(&g1,"237","570",5.83612741239,0);
    add_edge_graph(&g1,"237","647",9.24912664721,0);
    add_edge_graph(&g1,"238","824",5.7721045646,0);
    add_edge_graph(&g1,"238","899",5.45650734117,0);
    add_edge_graph(&g1,"238","340",6.62191010138,0);
    add_edge_graph(&g1,"240","304",3.93564014959,0);
    add_edge_graph(&g1,"241","874",2.41950576171,0);
    add_edge_graph(&g1,"241","414",2.99128776512,0);
    add_edge_graph(&g1,"242","596",3.77760621255,0);
    add_edge_graph(&g1,"243","808",6.52634333626,0);
    add_edge_graph(&g1,"244","386",6.17586619066,0);
    add_edge_graph(&g1,"244","647",9.44865788216,0);
    add_edge_graph(&g1,"244","527",8.84584953568,0);
    add_edge_graph(&g1,"244","786",3.43004669963,0);
    add_edge_graph(&g1,"244","659",9.7548191496,0);
    add_edge_graph(&g1,"244","691",4.8537804218,0);
    add_edge_graph(&g1,"244","587",8.06638284231,0);
    add_edge_graph(&g1,"244","596",6.05604919302,0);
    add_edge_graph(&g1,"244","982",9.16255604374,0);
    add_edge_graph(&g1,"244","736",7.2382670022,0);
    add_edge_graph(&g1,"244","849",1.59472399822,0);
    add_edge_graph(&g1,"244","488",0.64228511511,0);
    add_edge_graph(&g1,"244","874",5.68494812555,0);
    add_edge_graph(&g1,"244","744",5.53374208033,0);
    add_edge_graph(&g1,"244","498",6.12041706697,0);
    add_edge_graph(&g1,"244","531",1.26021419469,0);
    add_edge_graph(&g1,"244","888",7.01536899347,0);
    add_edge_graph(&g1,"244","890",8.14373110934,0);
    add_edge_graph(&g1,"245","521",3.62836538877,0);
    add_edge_graph(&g1,"245","314",1.24710783456,0);
    add_edge_graph(&g1,"245","681",6.74896492867,0);
    add_edge_graph(&g1,"246","888",5.34748368676,0);
    add_edge_graph(&g1,"246","874",3.89610523464,0);
    add_edge_graph(&g1,"248","389",6.90544432118,0);
    add_edge_graph(&g1,"249","717",0.0284426467019,0);
    add_edge_graph(&g1,"249","295",7.74446802927,0);
    add_edge_graph(&g1,"250","890",5.9787477964,0);
    add_edge_graph(&g1,"251","888",8.44171432444,0);
    add_edge_graph(&g1,"251","891",5.33250482275,0);
    add_edge_graph(&g1,"251","476",7.11229912895,0);
    add_edge_graph(&g1,"252","625",8.12262675284,0);
    add_edge_graph(&g1,"252","596",4.48202139771,0);
    add_edge_graph(&g1,"253","780",7.75470489021,0);
    add_edge_graph(&g1,"253","274",5.94006196698,0);
    add_edge_graph(&g1,"253","787",5.26543555315,0);
    add_edge_graph(&g1,"253","795",4.9847406842,0);
    add_edge_graph(&g1,"253","810",2.58396567531,0);
    add_edge_graph(&g1,"253","560",3.86928903245,0);
    add_edge_graph(&g1,"253","596",6.74782592181,0);
    add_edge_graph(&g1,"253","853",8.20847924781,0);
    add_edge_graph(&g1,"253","598",1.44042622444,0);
    add_edge_graph(&g1,"253","786",2.76072586605,0);
    add_edge_graph(&g1,"253","498",9.76600887659,0);
    add_edge_graph(&g1,"253","890",1.99762634574,0);
    add_edge_graph(&g1,"253","892",7.93541025414,0);
    add_edge_graph(&g1,"253","894",2.21801219174,0);
    add_edge_graph(&g1,"254","531",4.51257227325,0);
    add_edge_graph(&g1,"255","473",9.96195640182,0);
    add_edge_graph(&g1,"255","412",9.49543800176,0);
    add_edge_graph(&g1,"256","527",2.04065375151,0);
    add_edge_graph(&g1,"256","794",6.75916055738,0);
    add_edge_graph(&g1,"256","854",9.83262789538,0);
    add_edge_graph(&g1,"258","299",9.11501722798,0);
    add_edge_graph(&g1,"258","358",6.01043522481,0);
    add_edge_graph(&g1,"258","968",2.56305717337,0);
    add_edge_graph(&g1,"258","874",2.85272092839,0);
    add_edge_graph(&g1,"258","396",4.77571081262,0);
    add_edge_graph(&g1,"258","493",7.98133527946,0);
    add_edge_graph(&g1,"258","761",4.45706855942,0);
    add_edge_graph(&g1,"258","810",3.75817899122,0);
    add_edge_graph(&g1,"259","961",7.19026789607,0);
    add_edge_graph(&g1,"260","471",2.24865617443,0);
    add_edge_graph(&g1,"261","596",9.21482140923,0);
    add_edge_graph(&g1,"262","874",3.02491223982,0);
    add_edge_graph(&g1,"263","888",9.70024738215,0);
    add_edge_graph(&g1,"263","366",3.93304690469,0);
    add_edge_graph(&g1,"264","596",1.48029097055,0);
    add_edge_graph(&g1,"265","592",2.81241277415,0);
    add_edge_graph(&g1,"266","888",7.15983999715,0);
    add_edge_graph(&g1,"267","795",7.81771420052,0);
    add_edge_graph(&g1,"267","309",1.74162924806,0);
    add_edge_graph(&g1,"268","930",5.02119120783,0);
    add_edge_graph(&g1,"268","745",6.25572010746,0);
    add_edge_graph(&g1,"269","795",4.64212454693,0);
    add_edge_graph(&g1,"271","643",9.7396587894,0);
    add_edge_graph(&g1,"271","786",2.93190347424,0);
    add_edge_graph(&g1,"271","596",3.46188160539,0);
    add_edge_graph(&g1,"272","527",1.55161606515,0);
    add_edge_graph(&g1,"272","916",3.6144482701,0);
    add_edge_graph(&g1,"272","340",4.22140442759,0);
    add_edge_graph(&g1,"272","890",5.7563784224,0);
    add_edge_graph(&g1,"273","930",5.79475975388,0);
    add_edge_graph(&g1,"273","686",5.66725602325,0);
    add_edge_graph(&g1,"273","596",8.60460389181,0);
    add_edge_graph(&g1,"273","470",1.88001549634,0);
    add_edge_graph(&g1,"273","504",9.31453503789,0);
    add_edge_graph(&g1,"274","874",0.808985516676,0);
    add_edge_graph(&g1,"274","596",7.81255802989,0);
    add_edge_graph(&g1,"274","310",2.16868690806,0);
    add_edge_graph(&g1,"274","888",5.04441841031,0);
    add_edge_graph(&g1,"274","527",6.64655463373,0);
    add_edge_graph(&g1,"275","745",9.89909187076,0);
    add_edge_graph(&g1,"275","531",2.27314926295,0);
    add_edge_graph(&g1,"276","780",1.87808523137,0);
    add_edge_graph(&g1,"276","874",0.825276585296,0);
    add_edge_graph(&g1,"276","926",0.77490632399,0);
    add_edge_graph(&g1,"276","389",6.25892021915,0);
    add_edge_graph(&g1,"278","304",1.94791383098,0);
    add_edge_graph(&g1,"279","332",1.56563317684,0);
    add_edge_graph(&g1,"281","888",9.99246442441,0);
    add_edge_graph(&g1,"283","458",5.97501722588,0);
    add_edge_graph(&g1,"284","888",5.33562133943,0);
    add_edge_graph(&g1,"284","851",7.76864868546,0);
    add_edge_graph(&g1,"284","578",8.02552006095,0);
    add_edge_graph(&g1,"285","888",7.32935593422,0);
    add_edge_graph(&g1,"285","596",8.76658779477,0);
    add_edge_graph(&g1,"286","645",6.44927210964,0);
    add_edge_graph(&g1,"287","365",3.68123525426,0);
    add_edge_graph(&g1,"288","696",6.35714853387,0);
    add_edge_graph(&g1,"288","531",9.34047242287,0);
    add_edge_graph(&g1,"288","628",0.226659579512,0);
    add_edge_graph(&g1,"288","795",0.604789596567,0);
    add_edge_graph(&g1,"289","646",9.66626817971,0);
    add_edge_graph(&g1,"289","527",0.215534089048,0);
    add_edge_graph(&g1,"289","786",2.36313803282,0);
    add_edge_graph(&g1,"289","409",5.2046558587,0);
    add_edge_graph(&g1,"289","795",9.89622443374,0);
    add_edge_graph(&g1,"289","433",4.99052345258,0);
    add_edge_graph(&g1,"289","557",9.74998953281,0);
    add_edge_graph(&g1,"289","943",6.39637768539,0);
    add_edge_graph(&g1,"289","689",6.277637017,0);
    add_edge_graph(&g1,"289","563",1.43177495503,0);
    add_edge_graph(&g1,"289","823",2.69838920303,0);
    add_edge_graph(&g1,"289","824",5.37259862233,0);
    add_edge_graph(&g1,"289","803",3.72606317034,0);
    add_edge_graph(&g1,"289","319",8.4349706059,0);
    add_edge_graph(&g1,"289","961",5.47193809981,0);
    add_edge_graph(&g1,"289","327",6.29790237279,0);
    add_edge_graph(&g1,"289","968",1.57019048678,0);
    add_edge_graph(&g1,"289","854",0.624611924919,0);
    add_edge_graph(&g1,"289","473",9.11054883184,0);
    add_edge_graph(&g1,"289","996",3.97974548271,0);
    add_edge_graph(&g1,"289","614",9.39333629872,0);
    add_edge_graph(&g1,"289","999",8.5598628192,0);
    add_edge_graph(&g1,"289","874",2.77068822626,0);
    add_edge_graph(&g1,"289","979",8.90103032009,0);
    add_edge_graph(&g1,"289","888",8.60639103627,0);
    add_edge_graph(&g1,"289","596",2.25053619632,0);
    add_edge_graph(&g1,"289","890",9.18485999361,0);
    add_edge_graph(&g1,"291","348",0.281448110808,0);
    add_edge_graph(&g1,"292","888",2.5642101039,0);
    add_edge_graph(&g1,"293","547",2.15551953364,0);
    add_edge_graph(&g1,"293","457",8.2138139544,0);
    add_edge_graph(&g1,"293","874",5.95129519158,0);
    add_edge_graph(&g1,"293","890",9.29993229086,0);
    add_edge_graph(&g1,"293","734",3.55170516365,0);
    add_edge_graph(&g1,"294","596",4.69389849829,0);
    add_edge_graph(&g1,"294","295",1.51503935646,0);
    add_edge_graph(&g1,"295","642",1.04366209872,0);
    add_edge_graph(&g1,"295","780",7.57350082871,0);
    add_edge_graph(&g1,"295","397",0.389060325653,0);
    add_edge_graph(&g1,"295","527",5.84608623826,0);
    add_edge_graph(&g1,"295","786",7.04468977719,0);
    add_edge_graph(&g1,"295","796",3.27046999869,0);
    add_edge_graph(&g1,"295","543",5.12351410406,0);
    add_edge_graph(&g1,"295","304",4.61217407646,0);
    add_edge_graph(&g1,"295","547",9.17092660446,0);
    add_edge_graph(&g1,"295","566",5.49255649905,0);
    add_edge_graph(&g1,"295","810",8.56327298784,0);
    add_edge_graph(&g1,"295","954",8.20332654313,0);
    add_edge_graph(&g1,"295","316",3.678678058,0);
    add_edge_graph(&g1,"295","454",0.78989859356,0);
    add_edge_graph(&g1,"295","332",8.02301396135,0);
    add_edge_graph(&g1,"295","975",1.74433339996,0);
    add_edge_graph(&g1,"295","596",0.342889351575,0);
    add_edge_graph(&g1,"295","470",7.47788838844,0);
    add_edge_graph(&g1,"295","855",0.638661453878,0);
    add_edge_graph(&g1,"295","420",1.84905365306,0);
    add_edge_graph(&g1,"295","603",5.38518227593,0);
    add_edge_graph(&g1,"295","745",3.78344985957,0);
    add_edge_graph(&g1,"295","999",6.59629910986,0);
    add_edge_graph(&g1,"295","923",3.84025087255,0);
    add_edge_graph(&g1,"295","888",8.95266269043,0);
    add_edge_graph(&g1,"295","890",8.82898508685,0);
    add_edge_graph(&g1,"295","511",9.71402692827,0);
    add_edge_graph(&g1,"295","874",1.81545031087,0);
    add_edge_graph(&g1,"295","895",4.34748859889,0);
    add_edge_graph(&g1,"296","440",3.83248824966,0);
    add_edge_graph(&g1,"296","950",7.24641150397,0);
    add_edge_graph(&g1,"297","977",1.09092058091,0);
    add_edge_graph(&g1,"297","359",7.66215640608,0);
    add_edge_graph(&g1,"298","795",5.63479761574,0);
    add_edge_graph(&g1,"299","423",4.68122494844,0);
    add_edge_graph(&g1,"299","874",4.83656649764,0);
    add_edge_graph(&g1,"299","527",4.82264954568,0);
    add_edge_graph(&g1,"299","593",4.01787912184,0);
    add_edge_graph(&g1,"299","531",7.59269529074,0);
    add_edge_graph(&g1,"299","888",4.13746739089,0);
    add_edge_graph(&g1,"299","890",1.65682867613,0);
    add_edge_graph(&g1,"300","518",1.67636737901,0);
    add_edge_graph(&g1,"301","874",6.18274815148,0);
    add_edge_graph(&g1,"301","630",5.7972228485,0);
    add_edge_graph(&g1,"302","824",0.676928679147,0);
    add_edge_graph(&g1,"302","624",8.43971420762,0);
    add_edge_graph(&g1,"302","874",8.15562244556,0);
    add_edge_graph(&g1,"303","587",6.19668300895,0);
    add_edge_graph(&g1,"303","596",0.26810734333,0);
    add_edge_graph(&g1,"303","890",2.34482538172,0);
    add_edge_graph(&g1,"303","874",4.31385646289,0);
    add_edge_graph(&g1,"303","542",1.17489343931,0);
    add_edge_graph(&g1,"304","773",2.97923096313,0);
    add_edge_graph(&g1,"304","647",8.69916440095,0);
    add_edge_graph(&g1,"304","904",9.97834690639,0);
    add_edge_graph(&g1,"304","400",2.80260476825,0);
    add_edge_graph(&g1,"304","786",7.64276354353,0);
    add_edge_graph(&g1,"304","409",1.00495243339,0);
    add_edge_graph(&g1,"304","444",8.5961921705,0);
    add_edge_graph(&g1,"304","596",9.54792863937,0);
    add_edge_graph(&g1,"304","359",5.56054895624,0);
    add_edge_graph(&g1,"304","888",4.35089434911,0);
    add_edge_graph(&g1,"304","890",7.05599640334,0);
    add_edge_graph(&g1,"304","507",3.62377780758,0);
    add_edge_graph(&g1,"304","511",6.52592398252,0);
    add_edge_graph(&g1,"305","890",7.10246326194,0);
    add_edge_graph(&g1,"305","499",2.2170498887,0);
    add_edge_graph(&g1,"305","596",7.88774773223,0);
    add_edge_graph(&g1,"307","628",9.77057236487,0);
    add_edge_graph(&g1,"309","607",2.18656987405,0);
    add_edge_graph(&g1,"310","455",4.53347167557,0);
    add_edge_graph(&g1,"311","787",8.37604139647,0);
    add_edge_graph(&g1,"311","786",9.72828957924,0);
    add_edge_graph(&g1,"311","370",4.31383800278,0);
    add_edge_graph(&g1,"312","385",6.67569133305,0);
    add_edge_graph(&g1,"312","557",8.87917365935,0);
    add_edge_graph(&g1,"313","332",3.11380569123,0);
    add_edge_graph(&g1,"317","879",2.62830950867,0);
    add_edge_graph(&g1,"317","381",2.90615014166,0);
    add_edge_graph(&g1,"318","522",1.16635930339,0);
    add_edge_graph(&g1,"320","890",8.79329845082,0);
    add_edge_graph(&g1,"320","547",0.866864439641,0);
    add_edge_graph(&g1,"321","535",6.05012896145,0);
    add_edge_graph(&g1,"321","730",5.14363623825,0);
    add_edge_graph(&g1,"321","471",4.71548523509,0);
    add_edge_graph(&g1,"322","448",5.87371274452,0);
    add_edge_graph(&g1,"322","806",0.720853572777,0);
    add_edge_graph(&g1,"322","433",5.81209833857,0);
    add_edge_graph(&g1,"322","890",7.66954932136,0);
    add_edge_graph(&g1,"322","635",1.68440043999,0);
    add_edge_graph(&g1,"323","375",9.67936986266,0);
    add_edge_graph(&g1,"324","963",3.2409776119,0);
    add_edge_graph(&g1,"324","916",2.64149632786,0);
    add_edge_graph(&g1,"324","874",0.760287143849,0);
    add_edge_graph(&g1,"326","890",4.34307992484,0);
    add_edge_graph(&g1,"327","855",1.83660671718,0);
    add_edge_graph(&g1,"328","609",9.97641031445,0);
    add_edge_graph(&g1,"329","834",2.10152531644,0);
    add_edge_graph(&g1,"330","389",1.6337359613,0);
    add_edge_graph(&g1,"331","940",6.07766797499,0);
    add_edge_graph(&g1,"331","834",6.07312678019,0);
    add_edge_graph(&g1,"331","772",7.80882850781,0);
    add_edge_graph(&g1,"332","848",1.00077681011,0);
    add_edge_graph(&g1,"332","547",3.89540066755,0);
    add_edge_graph(&g1,"332","997",8.88838651316,0);
    add_edge_graph(&g1,"332","717",8.84744515667,0);
    add_edge_graph(&g1,"332","527",9.97195281804,0);
    add_edge_graph(&g1,"332","596",4.38941220798,0);
    add_edge_graph(&g1,"332","340",0.700250406873,0);
    add_edge_graph(&g1,"332","888",8.86946312522,0);
    add_edge_graph(&g1,"333","366",0.00243504452586,0);
    add_edge_graph(&g1,"335","596",4.75554778793,0);
    add_edge_graph(&g1,"335","455",4.16009834944,0);
    add_edge_graph(&g1,"336","890",1.59649253302,0);
    add_edge_graph(&g1,"336","619",1.46625465458,0);
    add_edge_graph(&g1,"337","557",5.45579254051,0);
    add_edge_graph(&g1,"338","409",6.22864558989,0);
    add_edge_graph(&g1,"338","789",4.30039030674,0);
    add_edge_graph(&g1,"339","968",3.30650152499,0);
    add_edge_graph(&g1,"339","596",6.95465202196,0);
    add_edge_graph(&g1,"339","458",5.42269696364,0);
    add_edge_graph(&g1,"340","596",6.07076379329,0);
    add_edge_graph(&g1,"341","874",9.14278475143,0);
    add_edge_graph(&g1,"343","888",9.13104859923,0);
    add_edge_graph(&g1,"347","890",6.08161679813,0);
    add_edge_graph(&g1,"347","874",6.79968533912,0);
    add_edge_graph(&g1,"348","521",6.4234937942,0);
    add_edge_graph(&g1,"348","761",6.17036606584,0);
    add_edge_graph(&g1,"349","905",9.42531554623,0);
    add_edge_graph(&g1,"349","527",6.02837388677,0);
    add_edge_graph(&g1,"349","533",7.56245450386,0);
    add_edge_graph(&g1,"349","470",6.40239142988,0);
    add_edge_graph(&g1,"349","890",0.827370978882,0);
    add_edge_graph(&g1,"349","874",9.67596894465,0);
    add_edge_graph(&g1,"351","874",6.97510725525,0);
    add_edge_graph(&g1,"352","899",3.02553271463,0);
    add_edge_graph(&g1,"354","855",3.71702460225,0);
    add_edge_graph(&g1,"355","835",7.95236639745,0);
    add_edge_graph(&g1,"357","739",9.9611754574,0);
    add_edge_graph(&g1,"358","531",0.272356210856,0);
    add_edge_graph(&g1,"359","423",3.99624112187,0);
    add_edge_graph(&g1,"359","874",6.2772773919,0);
    add_edge_graph(&g1,"359","596",3.48096451544,0);
    add_edge_graph(&g1,"360","890",4.70032192334,0);
    add_edge_graph(&g1,"361","660",6.47702521292,0);
    add_edge_graph(&g1,"361","853",7.00068468257,0);
    add_edge_graph(&g1,"361","527",0.73680196451,0);
    add_edge_graph(&g1,"362","888",2.56516458613,0);
    add_edge_graph(&g1,"362","993",1.78529621448,0);
    add_edge_graph(&g1,"362","898",5.2065632192,0);
    add_edge_graph(&g1,"363","874",0.125589598831,0);
    add_edge_graph(&g1,"364","888",5.9726059295,0);
    add_edge_graph(&g1,"365","546",5.58398649855,0);
    add_edge_graph(&g1,"365","890",5.12217406297,0);
    add_edge_graph(&g1,"366","384",6.61154043465,0);
    add_edge_graph(&g1,"366","774",3.43957528333,0);
    add_edge_graph(&g1,"366","623",6.50604783932,0);
    add_edge_graph(&g1,"366","527",4.00401330332,0);
    add_edge_graph(&g1,"366","912",5.22699238087,0);
    add_edge_graph(&g1,"366","786",3.26608134193,0);
    add_edge_graph(&g1,"366","539",5.17385323213,0);
    add_edge_graph(&g1,"366","547",1.43082151913,0);
    add_edge_graph(&g1,"366","938",3.32812870433,0);
    add_edge_graph(&g1,"366","557",6.13180866963,0);
    add_edge_graph(&g1,"366","888",7.74633004363,0);
    add_edge_graph(&g1,"366","946",8.31279989424,0);
    add_edge_graph(&g1,"366","696",5.41165224511,0);
    add_edge_graph(&g1,"366","697",6.49919596739,0);
    add_edge_graph(&g1,"366","667",8.50304956017,0);
    add_edge_graph(&g1,"366","459",4.49670330903,0);
    add_edge_graph(&g1,"366","596",6.82085717467,0);
    add_edge_graph(&g1,"366","476",4.76526019408,0);
    add_edge_graph(&g1,"366","874",5.75008367551,0);
    add_edge_graph(&g1,"366","500",9.18494718002,0);
    add_edge_graph(&g1,"366","632",2.40446273155,0);
    add_edge_graph(&g1,"366","890",3.13690595787,0);
    add_edge_graph(&g1,"367","689",9.27918683621,0);
    add_edge_graph(&g1,"368","888",3.89038309827,0);
    add_edge_graph(&g1,"369","968",6.59319311303,0);
    add_edge_graph(&g1,"369","770",3.78938067398,0);
    add_edge_graph(&g1,"370","993",9.49317685557,0);
    add_edge_graph(&g1,"370","596",7.82538887616,0);
    add_edge_graph(&g1,"371","940",2.93486213916,0);
    add_edge_graph(&g1,"371","511",5.10915449606,0);
    add_edge_graph(&g1,"372","528",7.16407781597,0);
    add_edge_graph(&g1,"372","890",5.79440427294,0);
    add_edge_graph(&g1,"372","645",6.58340477978,0);
    add_edge_graph(&g1,"373","409",8.70718673564,0);
    add_edge_graph(&g1,"373","890",8.02952567402,0);
    add_edge_graph(&g1,"374","874",6.57397941808,0);
    add_edge_graph(&g1,"376","921",6.74659488249,0);
    add_edge_graph(&g1,"376","868",8.97122782897,0);
    add_edge_graph(&g1,"377","890",5.23830450798,0);
    add_edge_graph(&g1,"377","404",1.35132496821,0);
    add_edge_graph(&g1,"378","874",2.56302056021,0);
    add_edge_graph(&g1,"379","596",5.36171995787,0);
    add_edge_graph(&g1,"381","547",7.42008649762,0);
    add_edge_graph(&g1,"381","458",1.46114031513,0);
    add_edge_graph(&g1,"381","874",2.42704370292,0);
    add_edge_graph(&g1,"381","786",8.19976254129,0);
    add_edge_graph(&g1,"381","594",8.57161092198,0);
    add_edge_graph(&g1,"381","824",1.59456330676,0);
    add_edge_graph(&g1,"381","388",1.28908550448,0);
    add_edge_graph(&g1,"381","890",6.05251514979,0);
    add_edge_graph(&g1,"381","892",3.52493761955,0);
    add_edge_graph(&g1,"383","874",3.24257251815,0);
    add_edge_graph(&g1,"384","416",0.813257971843,0);
    add_edge_graph(&g1,"384","425",7.96551899839,0);
    add_edge_graph(&g1,"384","874",0.690836076537,0);
    add_edge_graph(&g1,"384","536",7.26436962601,0);
    add_edge_graph(&g1,"384","890",5.07214908641,0);
    add_edge_graph(&g1,"384","954",0.102277807424,0);
    add_edge_graph(&g1,"385","747",5.73983638945,0);
    add_edge_graph(&g1,"385","582",8.91263895202,0);
    add_edge_graph(&g1,"385","585",6.48086410348,0);
    add_edge_graph(&g1,"385","890",7.44252043948,0);
    add_edge_graph(&g1,"386","450",7.52452977042,0);
    add_edge_graph(&g1,"386","961",3.81009041611,0);
    add_edge_graph(&g1,"386","874",8.2161007678,0);
    add_edge_graph(&g1,"386","624",9.0126039626,0);
    add_edge_graph(&g1,"386","502",4.81073524158,0);
    add_edge_graph(&g1,"386","404",4.62685407832,0);
    add_edge_graph(&g1,"386","890",7.93490046053,0);
    add_edge_graph(&g1,"386","746",5.89979793053,0);
    add_edge_graph(&g1,"386","798",5.44131453375,0);
    add_edge_graph(&g1,"387","761",0.126047940898,0);
    add_edge_graph(&g1,"387","596",6.44874707667,0);
    add_edge_graph(&g1,"388","769",6.54132644986,0);
    add_edge_graph(&g1,"388","874",1.56992989206,0);
    add_edge_graph(&g1,"388","527",2.99884214818,0);
    add_edge_graph(&g1,"388","496",3.04427804672,0);
    add_edge_graph(&g1,"388","831",6.22152349826,0);
    add_edge_graph(&g1,"389","772",8.02069578855,0);
    add_edge_graph(&g1,"389","521",9.27491958987,0);
    add_edge_graph(&g1,"389","527",8.73638967712,0);
    add_edge_graph(&g1,"389","786",0.616106877541,0);
    add_edge_graph(&g1,"389","686",0.227034544085,0);
    add_edge_graph(&g1,"389","547",9.84499937658,0);
    add_edge_graph(&g1,"389","940",0.743777255693,0);
    add_edge_graph(&g1,"389","573",5.50276125322,0);
    add_edge_graph(&g1,"389","708",6.3485070706,0);
    add_edge_graph(&g1,"389","716",0.996883740312,0);
    add_edge_graph(&g1,"389","717",3.74481233712,0);
    add_edge_graph(&g1,"389","597",7.31418391683,0);
    add_edge_graph(&g1,"389","874",6.97034920581,0);
    add_edge_graph(&g1,"389","679",3.00268134143,0);
    add_edge_graph(&g1,"389","595",9.06788717487,0);
    add_edge_graph(&g1,"389","532",1.95273034895,0);
    add_edge_graph(&g1,"389","890",2.31131340915,0);
    add_edge_graph(&g1,"390","795",4.15015210042,0);
    add_edge_graph(&g1,"391","695",8.7936237213,0);
    add_edge_graph(&g1,"391","898",5.6039619856,0);
    add_edge_graph(&g1,"391","554",4.13020984847,0);
    add_edge_graph(&g1,"392","936",1.14306423392,0);
    add_edge_graph(&g1,"392","810",6.25944458601,0);
    add_edge_graph(&g1,"392","890",6.81420677984,0);
    add_edge_graph(&g1,"393","888",9.89847743369,0);
    add_edge_graph(&g1,"395","628",5.39951708558,0);
    add_edge_graph(&g1,"396","943",5.6526373173,0);
    add_edge_graph(&g1,"397","643",9.89838271194,0);
    add_edge_graph(&g1,"397","521",6.46521914471,0);
    add_edge_graph(&g1,"397","786",1.31246195632,0);
    add_edge_graph(&g1,"397","951",1.56888792043,0);
    add_edge_graph(&g1,"397","536",6.21384273704,0);
    add_edge_graph(&g1,"397","890",2.6956543664,0);
    add_edge_graph(&g1,"397","527",0.211691779456,0);
    add_edge_graph(&g1,"397","511",6.77669727028,0);
    add_edge_graph(&g1,"398","874",0.0914766802368,0);
    add_edge_graph(&g1,"398","502",9.7898979816,0);
    add_edge_graph(&g1,"399","810",7.92436391658,0);
    add_edge_graph(&g1,"402","890",3.94280250628,0);
    add_edge_graph(&g1,"403","709",7.21833991554,0);
    add_edge_graph(&g1,"403","874",2.32827808121,0);
    add_edge_graph(&g1,"403","557",9.40138738119,0);
    add_edge_graph(&g1,"403","437",5.02764315562,0);
    add_edge_graph(&g1,"403","825",6.71459230321,0);
    add_edge_graph(&g1,"403","476",4.23521488973,0);
    add_edge_graph(&g1,"404","645",3.1466998362,0);
    add_edge_graph(&g1,"404","557",7.53354628574,0);
    add_edge_graph(&g1,"404","596",0.95286586012,0);
    add_edge_graph(&g1,"404","853",7.01168977135,0);
    add_edge_graph(&g1,"404","727",5.11715884358,0);
    add_edge_graph(&g1,"404","888",1.94866274797,0);
    add_edge_graph(&g1,"404","890",4.55824696292,0);
    add_edge_graph(&g1,"404","510",7.70283298994,0);
    add_edge_graph(&g1,"404","703",8.24247771868,0);
    add_edge_graph(&g1,"405","890",4.31466641719,0);
    add_edge_graph(&g1,"405","835",7.39243076745,0);
    add_edge_graph(&g1,"406","626",5.99437027894,0);
    add_edge_graph(&g1,"406","971",4.24304626573,0);
    add_edge_graph(&g1,"407","628",3.39925089214,0);
    add_edge_graph(&g1,"407","596",0.390907846621,0);
    add_edge_graph(&g1,"408","888",2.33292388175,0);
    add_edge_graph(&g1,"409","874",3.58538406292,0);
    add_edge_graph(&g1,"409","531",3.66288661359,0);
    add_edge_graph(&g1,"409","596",2.80194265886,0);
    add_edge_graph(&g1,"409","890",5.20528318111,0);
    add_edge_graph(&g1,"410","874",2.35388828312,0);
    add_edge_graph(&g1,"412","557",6.18748144671,0);
    add_edge_graph(&g1,"413","547",2.68881682789,0);
    add_edge_graph(&g1,"413","899",2.32685004008,0);
    add_edge_graph(&g1,"413","950",6.45037663914,0);
    add_edge_graph(&g1,"413","470",1.38676699559,0);
    add_edge_graph(&g1,"413","839",0.61875680247,0);
    add_edge_graph(&g1,"414","455",1.2665983538,0);
    add_edge_graph(&g1,"414","874",2.60923784816,0);
    add_edge_graph(&g1,"414","527",5.48833418865,0);
    add_edge_graph(&g1,"414","786",2.77537956884,0);
    add_edge_graph(&g1,"414","531",2.60862659813,0);
    add_edge_graph(&g1,"414","615",0.460379534921,0);
    add_edge_graph(&g1,"414","890",8.82478501709,0);
    add_edge_graph(&g1,"414","476",0.292509318501,0);
    add_edge_graph(&g1,"414","605",0.771779786762,0);
    add_edge_graph(&g1,"414","979",1.48027778272,0);
    add_edge_graph(&g1,"415","888",2.33231590986,0);
    add_edge_graph(&g1,"416","874",0.305154471987,0);
    add_edge_graph(&g1,"419","596",8.26579244779,0);
    add_edge_graph(&g1,"420","890",3.79508190945,0);
    add_edge_graph(&g1,"421","726",5.86286414108,0);
    add_edge_graph(&g1,"422","874",3.11221659951,0);
    add_edge_graph(&g1,"422","596",2.27399283657,0);
    add_edge_graph(&g1,"423","523",2.30332894551,0);
    add_edge_graph(&g1,"423","868",5.09494480255,0);
    add_edge_graph(&g1,"423","527",8.51293757718,0);
    add_edge_graph(&g1,"423","468",9.50406937892,0);
    add_edge_graph(&g1,"423","470",8.87347891892,0);
    add_edge_graph(&g1,"423","874",1.18132048857,0);
    add_edge_graph(&g1,"423","799",3.10189559247,0);
    add_edge_graph(&g1,"424","915",9.30869053882,0);
    add_edge_graph(&g1,"425","874",7.84753185838,0);
    add_edge_graph(&g1,"426","786",5.71601366582,0);
    add_edge_graph(&g1,"426","511",0.465534374432,0);
    add_edge_graph(&g1,"427","527",5.33154821314,0);
    add_edge_graph(&g1,"427","890",4.03056275832,0);
    add_edge_graph(&g1,"427","874",1.57396466762,0);
    add_edge_graph(&g1,"429","596",0.771643641595,0);
    add_edge_graph(&g1,"429","458",5.17628620028,0);
    add_edge_graph(&g1,"429","789",5.68073534649,0);
    add_edge_graph(&g1,"430","963",5.3222363011,0);
    add_edge_graph(&g1,"431","956",0.914632558655,0);
    add_edge_graph(&g1,"433","455",3.07462211937,0);
    add_edge_graph(&g1,"433","968",5.95080951736,0);
    add_edge_graph(&g1,"433","686",1.47878292514,0);
    add_edge_graph(&g1,"433","467",3.73044640788,0);
    add_edge_graph(&g1,"433","596",4.25700531809,0);
    add_edge_graph(&g1,"433","888",2.11721439129,0);
    add_edge_graph(&g1,"433","890",5.85358606813,0);
    add_edge_graph(&g1,"433","795",0.0530793491153,0);
    add_edge_graph(&g1,"433","874",1.49715064328,0);
    add_edge_graph(&g1,"434","547",5.71504067032,0);
    add_edge_graph(&g1,"436","888",4.4373062277,0);
    add_edge_graph(&g1,"436","836",1.67818809753,0);
    add_edge_graph(&g1,"438","511",8.43525544324,0);
    add_edge_graph(&g1,"439","824",8.60087274263,0);
    add_edge_graph(&g1,"439","596",9.40822961739,0);
    add_edge_graph(&g1,"440","705",1.04841754862,0);
    add_edge_graph(&g1,"440","596",9.71581131111,0);
    add_edge_graph(&g1,"440","890",3.74284341585,0);
    add_edge_graph(&g1,"441","786",4.24603124336,0);
    add_edge_graph(&g1,"442","874",7.90355836048,0);
    add_edge_graph(&g1,"443","557",3.96805424548,0);
    add_edge_graph(&g1,"444","513",5.92224825304,0);
    add_edge_graph(&g1,"445","841",5.77632814848,0);
    add_edge_graph(&g1,"445","535",7.28428280299,0);
    add_edge_graph(&g1,"445","569",8.92778549205,0);
    add_edge_graph(&g1,"446","745",3.71217472186,0);
    add_edge_graph(&g1,"446","458",2.50975548059,0);
    add_edge_graph(&g1,"446","596",5.71079609937,0);
    add_edge_graph(&g1,"446","729",8.1091153714,0);
    add_edge_graph(&g1,"447","874",9.55542474306,0);
    add_edge_graph(&g1,"447","531",9.39146609149,0);
    add_edge_graph(&g1,"447","685",5.63492358542,0);
    add_edge_graph(&g1,"448","888",0.446485072307,0);
    add_edge_graph(&g1,"449","874",9.39806607136,0);
    add_edge_graph(&g1,"449","918",1.58862833968,0);
    add_edge_graph(&g1,"450","780",0.0498034190889,0);
    add_edge_graph(&g1,"450","874",7.97308910192,0);
    add_edge_graph(&g1,"450","527",4.44995615765,0);
    add_edge_graph(&g1,"450","472",3.6871442728,0);
    add_edge_graph(&g1,"450","596",1.43477763131,0);
    add_edge_graph(&g1,"450","697",6.21641030207,0);
    add_edge_graph(&g1,"450","473",6.43540241635,0);
    add_edge_graph(&g1,"450","989",7.57370964374,0);
    add_edge_graph(&g1,"451","527",9.38023554126,0);
    add_edge_graph(&g1,"452","534",1.06698600663,0);
    add_edge_graph(&g1,"453","577",9.04439257292,0);
    add_edge_graph(&g1,"453","780",4.94494864181,0);
    add_edge_graph(&g1,"453","782",5.68152558477,0);
    add_edge_graph(&g1,"453","885",1.53790195171,0);
    add_edge_graph(&g1,"453","888",5.26639395372,0);
    add_edge_graph(&g1,"454","527",6.38327259842,0);
    add_edge_graph(&g1,"455","756",8.40781025255,0);
    add_edge_graph(&g1,"455","745",6.79362019592,0);
    add_edge_graph(&g1,"455","874",2.17304726923,0);
    add_edge_graph(&g1,"455","711",6.57128099079,0);
    add_edge_graph(&g1,"455","596",8.97428162625,0);
    add_edge_graph(&g1,"455","786",3.72863992328,0);
    add_edge_graph(&g1,"455","883",8.41239204261,0);
    add_edge_graph(&g1,"455","903",4.29123323823,0);
    add_edge_graph(&g1,"455","888",5.56194537719,0);
    add_edge_graph(&g1,"455","890",1.71933889031,0);
    add_edge_graph(&g1,"455","527",4.81455297864,0);
    add_edge_graph(&g1,"455","570",3.54990480688,0);
    add_edge_graph(&g1,"457","471",3.7976872017,0);
    add_edge_graph(&g1,"458","778",6.34180270673,0);
    add_edge_graph(&g1,"458","874",9.21928197174,0);
    add_edge_graph(&g1,"458","761",9.81977490538,0);
    add_edge_graph(&g1,"458","786",1.27425923711,0);
    add_edge_graph(&g1,"458","596",9.48163054196,0);
    add_edge_graph(&g1,"458","950",5.71373810228,0);
    add_edge_graph(&g1,"458","495",7.12782246387,0);
    add_edge_graph(&g1,"458","888",5.98200803995,0);
    add_edge_graph(&g1,"458","527",0.0920835883704,0);
    add_edge_graph(&g1,"458","890",3.17631486209,0);
    add_edge_graph(&g1,"458","863",0.731758805762,0);
    add_edge_graph(&g1,"459","874",3.85801183172,0);
    add_edge_graph(&g1,"459","786",9.18132171323,0);
    add_edge_graph(&g1,"459","890",4.45511593284,0);
    add_edge_graph(&g1,"459","763",1.62751119349,0);
    add_edge_graph(&g1,"459","970",7.38169389755,0);
    add_edge_graph(&g1,"460","596",6.05902973344,0);
    add_edge_graph(&g1,"461","522",0.347820399395,0);
    add_edge_graph(&g1,"462","499",8.51406119589,0);
    add_edge_graph(&g1,"463","596",7.1564074209,0);
    add_edge_graph(&g1,"465","596",5.37433049261,0);
    add_edge_graph(&g1,"466","890",5.24623130879,0);
    add_edge_graph(&g1,"467","968",5.06898326122,0);
    add_edge_graph(&g1,"467","904",3.65164044936,0);
    add_edge_graph(&g1,"467","596",9.55087719086,0);
    add_edge_graph(&g1,"467","511",6.00501020462,0);
    add_edge_graph(&g1,"468","888",3.40563741701,0);
    add_edge_graph(&g1,"470","512",6.7046626794,0);
    add_edge_graph(&g1,"470","518",0.082224336443,0);
    add_edge_graph(&g1,"470","911",6.41965110299,0);
    add_edge_graph(&g1,"470","787",8.07840569055,0);
    add_edge_graph(&g1,"470","660",6.16445813509,0);
    add_edge_graph(&g1,"470","955",0.776692642965,0);
    add_edge_graph(&g1,"470","831",3.09095769597,0);
    add_edge_graph(&g1,"470","963",4.85410511023,0);
    add_edge_graph(&g1,"470","965",1.28350441728,0);
    add_edge_graph(&g1,"470","545",3.38616051458,0);
    add_edge_graph(&g1,"470","596",2.04181308306,0);
    add_edge_graph(&g1,"470","473",4.54872810351,0);
    add_edge_graph(&g1,"470","527",2.32982558209,0);
    add_edge_graph(&g1,"470","874",4.39903249253,0);
    add_edge_graph(&g1,"470","890",4.96705507619,0);
    add_edge_graph(&g1,"471","874",1.87611553765,0);
    add_edge_graph(&g1,"471","623",6.54130389281,0);
    add_edge_graph(&g1,"471","916",7.89946491325,0);
    add_edge_graph(&g1,"472","786",7.43405607799,0);
    add_edge_graph(&g1,"473","652",1.91485070258,0);
    add_edge_graph(&g1,"473","527",3.22133371192,0);
    add_edge_graph(&g1,"473","531",7.3875905665,0);
    add_edge_graph(&g1,"473","836",0.165654854825,0);
    add_edge_graph(&g1,"473","795",7.33111949625,0);
    add_edge_graph(&g1,"473","797",0.582160946669,0);
    add_edge_graph(&g1,"473","799",4.77284892587,0);
    add_edge_graph(&g1,"473","950",9.75454220401,0);
    add_edge_graph(&g1,"473","951",7.92045430358,0);
    add_edge_graph(&g1,"473","697",3.711105579,0);
    add_edge_graph(&g1,"473","954",1.69946051703,0);
    add_edge_graph(&g1,"473","832",5.32733776109,0);
    add_edge_graph(&g1,"473","963",4.45774058035,0);
    add_edge_graph(&g1,"473","888",4.31362774644,0);
    add_edge_graph(&g1,"473","547",5.74096998483,0);
    add_edge_graph(&g1,"473","596",4.5720083228,0);
    add_edge_graph(&g1,"473","855",9.72715811658,0);
    add_edge_graph(&g1,"473","866",3.68545100051,0);
    add_edge_graph(&g1,"473","874",2.82940809641,0);
    add_edge_graph(&g1,"473","761",2.23794767261,0);
    add_edge_graph(&g1,"473","890",6.31219214275,0);
    add_edge_graph(&g1,"474","832",0.288701238557,0);
    add_edge_graph(&g1,"474","557",0.670601713272,0);
    add_edge_graph(&g1,"474","527",5.34547472791,0);
    add_edge_graph(&g1,"474","596",8.87075098255,0);
    add_edge_graph(&g1,"474","756",6.5716247094,0);
    add_edge_graph(&g1,"474","890",0.682186025974,0);
    add_edge_graph(&g1,"474","874",4.45400692467,0);
    add_edge_graph(&g1,"475","527",5.46294588025,0);
    add_edge_graph(&g1,"476","641",9.28733981044,0);
    add_edge_graph(&g1,"476","791",1.08219623367,0);
    add_edge_graph(&g1,"476","950",5.5466960274,0);
    add_edge_graph(&g1,"476","704",1.72309944479,0);
    add_edge_graph(&g1,"476","874",2.91564239743,0);
    add_edge_graph(&g1,"476","498",5.30657447559,0);
    add_edge_graph(&g1,"476","888",8.15458577286,0);
    add_edge_graph(&g1,"476","890",0.0646957261054,0);
    add_edge_graph(&g1,"476","638",7.84460557653,0);
    add_edge_graph(&g1,"477","888",2.68004076041,0);
    add_edge_graph(&g1,"477","890",3.03762579367,0);
    add_edge_graph(&g1,"479","772",6.90944299891,0);
    add_edge_graph(&g1,"480","511",3.8520223691,0);
    add_edge_graph(&g1,"481","557",1.57095812304,0);
    add_edge_graph(&g1,"482","596",4.37695201771,0);
    add_edge_graph(&g1,"483","970",8.64966183154,0);
    add_edge_graph(&g1,"487","874",2.73786604095,0);
    add_edge_graph(&g1,"487","830",8.48140885518,0);
    add_edge_graph(&g1,"487","855",2.92150527684,0);
    add_edge_graph(&g1,"488","707",5.73344326142,0);
    add_edge_graph(&g1,"489","890",1.46528102463,0);
    add_edge_graph(&g1,"491","547",8.69059944489,0);
    add_edge_graph(&g1,"491","596",8.87023369068,0);
    add_edge_graph(&g1,"492","888",0.587770069337,0);
    add_edge_graph(&g1,"493","780",9.27910285808,0);
    add_edge_graph(&g1,"493","888",4.83515098934,0);
    add_edge_graph(&g1,"493","890",9.30753040143,0);
    add_edge_graph(&g1,"495","898",7.43026974145,0);
    add_edge_graph(&g1,"495","813",8.43102408651,0);
    add_edge_graph(&g1,"495","596",0.867167102069,0);
    add_edge_graph(&g1,"495","889",0.708608174048,0);
    add_edge_graph(&g1,"497","596",2.72977364473,0);
    add_edge_graph(&g1,"498","548",2.82314110379,0);
    add_edge_graph(&g1,"498","874",9.53377667227,0);
    add_edge_graph(&g1,"498","824",7.03285201156,0);
    add_edge_graph(&g1,"498","527",1.1497698783,0);
    add_edge_graph(&g1,"498","593",0.491246116714,0);
    add_edge_graph(&g1,"498","888",5.3969321111,0);
    add_edge_graph(&g1,"498","596",1.56555035904,0);
    add_edge_graph(&g1,"498","890",0.658093055094,0);
    add_edge_graph(&g1,"499","596",2.72064120079,0);
    add_edge_graph(&g1,"499","527",5.43707545956,0);
    add_edge_graph(&g1,"500","562",8.10348863246,0);
    add_edge_graph(&g1,"500","675",2.47550642941,0);
    add_edge_graph(&g1,"501","797",4.8742940726,0);
    add_edge_graph(&g1,"502","911",6.31060150192,0);
    add_edge_graph(&g1,"502","786",1.6284299124,0);
    add_edge_graph(&g1,"502","939",3.78505186794,0);
    add_edge_graph(&g1,"502","824",8.55481838331,0);
    add_edge_graph(&g1,"502","573",2.49792173117,0);
    add_edge_graph(&g1,"502","709",1.49446439104,0);
    add_edge_graph(&g1,"502","596",8.14636302833,0);
    add_edge_graph(&g1,"502","874",0.558440725403,0);
    add_edge_graph(&g1,"502","888",4.68675614143,0);
    add_edge_graph(&g1,"502","890",6.40925006114,0);
    add_edge_graph(&g1,"503","596",9.23926825843,0);
    add_edge_graph(&g1,"503","535",8.60069772609,0);
    add_edge_graph(&g1,"504","596",4.66469891268,0);
    add_edge_graph(&g1,"505","888",6.33210182543,0);
    add_edge_graph(&g1,"505","705",3.46300524911,0);
    add_edge_graph(&g1,"505","527",1.64249983046,0);
    add_edge_graph(&g1,"509","652",5.57687778144,0);
    add_edge_graph(&g1,"510","890",4.96300668834,0);
    add_edge_graph(&g1,"511","774",6.58491729889,0);
    add_edge_graph(&g1,"511","652",2.44586283711,0);
    add_edge_graph(&g1,"511","527",9.43872223139,0);
    add_edge_graph(&g1,"511","786",9.82275124412,0);
    add_edge_graph(&g1,"511","531",8.46900615994,0);
    add_edge_graph(&g1,"511","795",4.80194749548,0);
    add_edge_graph(&g1,"511","553",6.50728132745,0);
    add_edge_graph(&g1,"511","695",1.44523464072,0);
    add_edge_graph(&g1,"511","824",5.46612431997,0);
    add_edge_graph(&g1,"511","701",8.91649470747,0);
    add_edge_graph(&g1,"511","963",3.26498618336,0);
    add_edge_graph(&g1,"511","780",2.24718446132,0);
    add_edge_graph(&g1,"511","590",1.81711988305,0);
    add_edge_graph(&g1,"511","596",2.01547556152,0);
    add_edge_graph(&g1,"511","738",2.51881348504,0);
    add_edge_graph(&g1,"511","977",3.9987501209,0);
    add_edge_graph(&g1,"511","874",1.50240859736,0);
    add_edge_graph(&g1,"511","888",9.00786769618,0);
    add_edge_graph(&g1,"511","890",0.371136637977,0);
    add_edge_graph(&g1,"513","689",7.67992460449,0);
    add_edge_graph(&g1,"516","585",6.71294956782,0);
    add_edge_graph(&g1,"518","562",8.6339898248,0);
    add_edge_graph(&g1,"519","890",3.26716418552,0);
    add_edge_graph(&g1,"520","874",3.52199959995,0);
    add_edge_graph(&g1,"521","642",9.98545987521,0);
    add_edge_graph(&g1,"521","874",6.63645337777,0);
    add_edge_graph(&g1,"521","526",5.92192910365,0);
    add_edge_graph(&g1,"521","527",9.84105626385,0);
    add_edge_graph(&g1,"521","890",7.36304942645,0);
    add_edge_graph(&g1,"521","923",7.30126315006,0);
    add_edge_graph(&g1,"521","810",0.157600449372,0);
    add_edge_graph(&g1,"522","817",8.75983307536,0);
    add_edge_graph(&g1,"523","632",6.08319297778,0);
    add_edge_graph(&g1,"524","890",1.46317554767,0);
    add_edge_graph(&g1,"524","786",7.6132460289,0);
    add_edge_graph(&g1,"524","557",6.99975576514,0);
    add_edge_graph(&g1,"525","786",2.57865585269,0);
    add_edge_graph(&g1,"527","531",0.132604963645,0);
    add_edge_graph(&g1,"527","557",6.210605269,0);
    add_edge_graph(&g1,"527","565",1.67455553522,0);
    add_edge_graph(&g1,"527","577",5.64679431565,0);
    add_edge_graph(&g1,"527","694",3.95838138904,0);
    add_edge_graph(&g1,"527","596",7.04960733678,0);
    add_edge_graph(&g1,"527","786",0.244945223259,0);
    add_edge_graph(&g1,"527","653",2.35322134464,0);
    add_edge_graph(&g1,"527","654",3.04076624037,0);
    add_edge_graph(&g1,"527","963",5.96361418753,0);
    add_edge_graph(&g1,"527","668",0.0444873175453,0);
    add_edge_graph(&g1,"527","685",5.6945353055,0);
    add_edge_graph(&g1,"527","689",2.69336769915,0);
    add_edge_graph(&g1,"527","697",5.56986604186,0);
    add_edge_graph(&g1,"527","700",1.51585954958,0);
    add_edge_graph(&g1,"527","702",1.86884257211,0);
    add_edge_graph(&g1,"527","704",9.06376415196,0);
    add_edge_graph(&g1,"527","705",7.70041076834,0);
    add_edge_graph(&g1,"527","709",5.4130004758,0);
    add_edge_graph(&g1,"527","547",1.44757129549,0);
    add_edge_graph(&g1,"527","739",2.55699739773,0);
    add_edge_graph(&g1,"527","740",5.05241007706,0);
    add_edge_graph(&g1,"527","744",6.4826913431,0);
    add_edge_graph(&g1,"527","761",9.11226413165,0);
    add_edge_graph(&g1,"527","766",3.64828742385,0);
    add_edge_graph(&g1,"527","772",2.83231705524,0);
    add_edge_graph(&g1,"527","774",9.82986284475,0);
    add_edge_graph(&g1,"527","780",8.37481210312,0);
    add_edge_graph(&g1,"527","729",4.86628174784,0);
    add_edge_graph(&g1,"527","795",4.72829011004,0);
    add_edge_graph(&g1,"527","986",3.67109831504,0);
    add_edge_graph(&g1,"527","987",4.86061403446,0);
    add_edge_graph(&g1,"527","810",8.46143653318,0);
    add_edge_graph(&g1,"527","821",3.19685936695,0);
    add_edge_graph(&g1,"527","823",5.94728494796,0);
    add_edge_graph(&g1,"527","826",2.13745385355,0);
    add_edge_graph(&g1,"527","835",8.53757456397,0);
    add_edge_graph(&g1,"527","838",9.54664037,0);
    add_edge_graph(&g1,"527","853",3.44025469446,0);
    add_edge_graph(&g1,"527","855",0.116422441535,0);
    add_edge_graph(&g1,"527","868",7.33908240447,0);
    add_edge_graph(&g1,"527","874",9.36846951231,0);
    add_edge_graph(&g1,"527","876",1.65686147057,0);
    add_edge_graph(&g1,"527","888",2.26521605816,0);
    add_edge_graph(&g1,"527","890",3.0396489742,0);
    add_edge_graph(&g1,"527","895",8.01418388916,0);
    add_edge_graph(&g1,"527","559",2.18722996588,0);
    add_edge_graph(&g1,"527","955",9.1914615485,0);
    add_edge_graph(&g1,"527","958",7.72699552843,0);
    add_edge_graph(&g1,"527","968",7.92229534654,0);
    add_edge_graph(&g1,"527","975",1.76454546488,0);
    add_edge_graph(&g1,"527","590",4.85158880371,0);
    add_edge_graph(&g1,"527","983",1.24289607161,0);
    add_edge_graph(&g1,"527","680",6.46929159427,0);
    add_edge_graph(&g1,"528","874",3.59128279118,0);
    add_edge_graph(&g1,"528","786",1.34176138665,0);
    add_edge_graph(&g1,"528","888",7.47382587963,0);
    add_edge_graph(&g1,"529","531",5.53485445422,0);
    add_edge_graph(&g1,"529","890",8.15766579651,0);
    add_edge_graph(&g1,"530","968",7.19209352474,0);
    add_edge_graph(&g1,"530","874",3.61230922693,0);
    add_edge_graph(&g1,"530","824",2.58251210083,0);
    add_edge_graph(&g1,"530","761",3.52473119961,0);
    add_edge_graph(&g1,"531","901",0.109238487688,0);
    add_edge_graph(&g1,"531","774",9.62609055022,0);
    add_edge_graph(&g1,"531","661",8.8482555149,0);
    add_edge_graph(&g1,"531","786",6.72651919244,0);
    add_edge_graph(&g1,"531","916",1.98695825395,0);
    add_edge_graph(&g1,"531","923",5.32260255452,0);
    add_edge_graph(&g1,"531","549",5.01573835459,0);
    add_edge_graph(&g1,"531","682",2.32632744621,0);
    add_edge_graph(&g1,"531","547",0.434805068433,0);
    add_edge_graph(&g1,"531","951",6.29753678085,0);
    add_edge_graph(&g1,"531","696",6.03645380478,0);
    add_edge_graph(&g1,"531","699",6.69999289841,0);
    add_edge_graph(&g1,"531","963",6.18523310849,0);
    add_edge_graph(&g1,"531","839",9.1738606891,0);
    add_edge_graph(&g1,"531","596",3.64964587312,0);
    add_edge_graph(&g1,"531","855",6.61514589393,0);
    add_edge_graph(&g1,"531","851",7.52636606787,0);
    add_edge_graph(&g1,"531","861",2.42069461331,0);
    add_edge_graph(&g1,"531","606",4.35093792801,0);
    add_edge_graph(&g1,"531","745",3.78915536748,0);
    add_edge_graph(&g1,"531","874",2.97510201843,0);
    add_edge_graph(&g1,"531","595",4.23302969122,0);
    add_edge_graph(&g1,"531","888",5.7334252685,0);
    add_edge_graph(&g1,"531","890",9.34690494855,0);
    add_edge_graph(&g1,"531","686",3.1786610965,0);
    add_edge_graph(&g1,"535","628",4.98420944953,0);
    add_edge_graph(&g1,"538","922",3.73898793547,0);
    add_edge_graph(&g1,"540","596",2.94604554339,0);
    add_edge_graph(&g1,"541","874",8.03309254736,0);
    add_edge_graph(&g1,"541","702",8.62503510294,0);
    add_edge_graph(&g1,"543","821",5.05579638333,0);
    add_edge_graph(&g1,"545","890",1.25821202056,0);
    add_edge_graph(&g1,"546","963",0.464217697777,0);
    add_edge_graph(&g1,"547","898",1.8100984409,0);
    add_edge_graph(&g1,"547","780",0.911524759318,0);
    add_edge_graph(&g1,"547","785",0.262638465421,0);
    add_edge_graph(&g1,"547","786",2.55155398486,0);
    add_edge_graph(&g1,"547","814",4.60190823629,0);
    add_edge_graph(&g1,"547","673",3.19113680524,0);
    add_edge_graph(&g1,"547","930",3.13781886562,0);
    add_edge_graph(&g1,"547","552",1.89611614034,0);
    add_edge_graph(&g1,"547","554",4.19002382174,0);
    add_edge_graph(&g1,"547","939",8.61528183848,0);
    add_edge_graph(&g1,"547","935",4.9744086977,0);
    add_edge_graph(&g1,"547","950",6.8168303795,0);
    add_edge_graph(&g1,"547","954",6.27169538398,0);
    add_edge_graph(&g1,"547","828",7.48307158067,0);
    add_edge_graph(&g1,"547","708",9.40115833584,0);
    add_edge_graph(&g1,"547","840",9.1135345799,0);
    add_edge_graph(&g1,"547","587",7.5316033115,0);
    add_edge_graph(&g1,"547","851",5.52948947467,0);
    add_edge_graph(&g1,"547","596",8.28652460538,0);
    add_edge_graph(&g1,"547","858",5.96658487135,0);
    add_edge_graph(&g1,"547","549",9.54768163891,0);
    add_edge_graph(&g1,"547","745",7.25428071545,0);
    add_edge_graph(&g1,"547","874",9.453527542,0);
    add_edge_graph(&g1,"547","787",2.53347547725,0);
    add_edge_graph(&g1,"547","888",1.92125212294,0);
    add_edge_graph(&g1,"547","890",1.9988835481,0);
    add_edge_graph(&g1,"549","816",6.17151464532,0);
    add_edge_graph(&g1,"549","786",7.33537747659,0);
    add_edge_graph(&g1,"550","910",5.36492041576,0);
    add_edge_graph(&g1,"551","853",3.64143926537,0);
    add_edge_graph(&g1,"552","948",4.31494430737,0);
    add_edge_graph(&g1,"552","596",7.03341209572,0);
    add_edge_graph(&g1,"552","785",7.85921760979,0);
    add_edge_graph(&g1,"553","739",4.0343512577,0);
    add_edge_graph(&g1,"553","557",0.758518659265,0);
    add_edge_graph(&g1,"554","609",5.79984134421,0);
    add_edge_graph(&g1,"554","874",2.49081885445,0);
    add_edge_graph(&g1,"554","985",7.04454312537,0);
    add_edge_graph(&g1,"555","708",5.85265753087,0);
    add_edge_graph(&g1,"555","838",2.89867949548,0);
    add_edge_graph(&g1,"557","769",3.8175413814,0);
    add_edge_graph(&g1,"557","786",3.97893645206,0);
    add_edge_graph(&g1,"557","674",0.77116314728,0);
    add_edge_graph(&g1,"557","685",3.10608750628,0);
    add_edge_graph(&g1,"557","689",9.91668245111,0);
    add_edge_graph(&g1,"557","949",7.18217826019,0);
    add_edge_graph(&g1,"557","582",6.41012013012,0);
    add_edge_graph(&g1,"557","795",5.12397903134,0);
    add_edge_graph(&g1,"557","585",8.62871194761,0);
    add_edge_graph(&g1,"557","717",3.19359142886,0);
    add_edge_graph(&g1,"557","850",9.34642614939,0);
    add_edge_graph(&g1,"557","596",1.22962737795,0);
    add_edge_graph(&g1,"557","667",8.05783876299,0);
    add_edge_graph(&g1,"557","868",5.55806570541,0);
    add_edge_graph(&g1,"557","874",5.24506047145,0);
    add_edge_graph(&g1,"557","888",3.04969172723,0);
    add_edge_graph(&g1,"557","890",4.34397633153,0);
    add_edge_graph(&g1,"557","899",2.88402971263,0);
    add_edge_graph(&g1,"558","916",8.82556553625,0);
    add_edge_graph(&g1,"560","954",2.47529459101,0);
    add_edge_graph(&g1,"560","775",5.71800712669,0);
    add_edge_graph(&g1,"561","628",6.5639346189,0);
    add_edge_graph(&g1,"563","888",1.5065389344,0);
    add_edge_graph(&g1,"564","890",4.79511139879,0);
    add_edge_graph(&g1,"565","968",5.78508288872,0);
    add_edge_graph(&g1,"566","888",7.54902917764,0);
    add_edge_graph(&g1,"568","925",1.72336526373,0);
    add_edge_graph(&g1,"570","689",7.18361515857,0);
    add_edge_graph(&g1,"573","928",7.78384653476,0);
    add_edge_graph(&g1,"574","890",4.68336989799,0);
    add_edge_graph(&g1,"575","960",1.76191773791,0);
    add_edge_graph(&g1,"576","596",3.43591823531,0);
    add_edge_graph(&g1,"577","874",9.44579049401,0);
    add_edge_graph(&g1,"578","583",8.39131586016,0);
    add_edge_graph(&g1,"578","938",5.3280665509,0);
    add_edge_graph(&g1,"578","890",4.57877643001,0);
    add_edge_graph(&g1,"578","874",3.05244322574,0);
    add_edge_graph(&g1,"579","874",9.13342840901,0);
    add_edge_graph(&g1,"579","835",7.99748339773,0);
    add_edge_graph(&g1,"580","840",8.41894565013,0);
    add_edge_graph(&g1,"583","761",5.55948113084,0);
    add_edge_graph(&g1,"584","890",4.32309525281,0);
    add_edge_graph(&g1,"584","874",2.40367636971,0);
    add_edge_graph(&g1,"584","589",2.32938114569,0);
    add_edge_graph(&g1,"585","963",5.73084877372,0);
    add_edge_graph(&g1,"586","874",4.3380270066,0);
    add_edge_graph(&g1,"587","874",9.40048911308,0);
    add_edge_graph(&g1,"588","675",7.95665628361,0);
    add_edge_graph(&g1,"588","890",5.88327585657,0);
    add_edge_graph(&g1,"588","874",7.93049846476,0);
    add_edge_graph(&g1,"589","888",3.43223622756,0);
    add_edge_graph(&g1,"589","890",2.74639960363,0);
    add_edge_graph(&g1,"590","888",9.57112609075,0);
    add_edge_graph(&g1,"591","596",7.08188817126,0);
    add_edge_graph(&g1,"591","797",6.50101093925,0);
    add_edge_graph(&g1,"595","874",8.53459656652,0);
    add_edge_graph(&g1,"595","615",4.84946910784,0);
    add_edge_graph(&g1,"596","921",6.9503947268,0);
    add_edge_graph(&g1,"596","773",1.57364794387,0);
    add_edge_graph(&g1,"596","786",3.86162602527,0);
    add_edge_graph(&g1,"596","667",0.444784410722,0);
    add_edge_graph(&g1,"596","597",0.987631249424,0);
    add_edge_graph(&g1,"596","605",7.83116544528,0);
    add_edge_graph(&g1,"596","612",9.84197444555,0);
    add_edge_graph(&g1,"596","620",8.80466618847,0);
    add_edge_graph(&g1,"596","643",7.48988720582,0);
    add_edge_graph(&g1,"596","652",5.73558232995,0);
    add_edge_graph(&g1,"596","660",8.86140658985,0);
    add_edge_graph(&g1,"596","675",5.56696205805,0);
    add_edge_graph(&g1,"596","797",4.32940352194,0);
    add_edge_graph(&g1,"596","689",9.08588233775,0);
    add_edge_graph(&g1,"596","697",8.11652824287,0);
    add_edge_graph(&g1,"596","701",0.237847749726,0);
    add_edge_graph(&g1,"596","702",1.67088606575,0);
    add_edge_graph(&g1,"596","709",0.196784065621,0);
    add_edge_graph(&g1,"596","718",5.80845414321,0);
    add_edge_graph(&g1,"596","728",6.46927310062,0);
    add_edge_graph(&g1,"596","975",8.16277765806,0);
    add_edge_graph(&g1,"596","736",9.70482740689,0);
    add_edge_graph(&g1,"596","945",7.25902078677,0);
    add_edge_graph(&g1,"596","741",7.55993675707,0);
    add_edge_graph(&g1,"596","760",2.59537123621,0);
    add_edge_graph(&g1,"596","761",9.2004619255,0);
    add_edge_graph(&g1,"596","780",9.55818985804,0);
    add_edge_graph(&g1,"596","795",8.96364666387,0);
    add_edge_graph(&g1,"596","800",2.30096384385,0);
    add_edge_graph(&g1,"596","987",1.7485268758,0);
    add_edge_graph(&g1,"596","805",0.0501413678679,0);
    add_edge_graph(&g1,"596","808",4.42733773489,0);
    add_edge_graph(&g1,"596","810",3.23992411121,0);
    add_edge_graph(&g1,"596","818",5.8463921226,0);
    add_edge_graph(&g1,"596","822",8.6946058654,0);
    add_edge_graph(&g1,"596","824",7.1123802482,0);
    add_edge_graph(&g1,"596","851",1.4375449819,0);
    add_edge_graph(&g1,"596","845",6.83758891905,0);
    add_edge_graph(&g1,"596","850",5.24727531973,0);
    add_edge_graph(&g1,"596","855",8.1771884844,0);
    add_edge_graph(&g1,"596","857",5.68827121746,0);
    add_edge_graph(&g1,"596","859",9.1078441143,0);
    add_edge_graph(&g1,"596","868",4.51966586466,0);
    add_edge_graph(&g1,"596","874",5.16442144855,0);
    add_edge_graph(&g1,"596","876",1.00236886341,0);
    add_edge_graph(&g1,"596","880",2.57519358772,0);
    add_edge_graph(&g1,"596","888",4.16067497088,0);
    add_edge_graph(&g1,"596","890",7.32454426162,0);
    add_edge_graph(&g1,"596","907",1.70032138656,0);
    add_edge_graph(&g1,"596","908",1.94710538316,0);
    add_edge_graph(&g1,"596","915",7.80756289591,0);
    add_edge_graph(&g1,"596","920",8.32737901214,0);
    add_edge_graph(&g1,"596","935",0.460707730031,0);
    add_edge_graph(&g1,"596","937",8.57924769104,0);
    add_edge_graph(&g1,"596","940",5.3907521022,0);
    add_edge_graph(&g1,"596","986",6.4783030138,0);
    add_edge_graph(&g1,"596","756",4.36083617844,0);
    add_edge_graph(&g1,"596","954",0.862877482781,0);
    add_edge_graph(&g1,"596","957",7.50745949653,0);
    add_edge_graph(&g1,"596","963",3.10708321021,0);
    add_edge_graph(&g1,"596","989",0.118937953737,0);
    add_edge_graph(&g1,"596","992",2.39215350847,0);
    add_edge_graph(&g1,"596","996",3.53476047962,0);
    add_edge_graph(&g1,"596","934",1.65144518436,0);
    add_edge_graph(&g1,"596","682",8.28096683617,0);
    add_edge_graph(&g1,"597","757",3.07543884383,0);
    add_edge_graph(&g1,"598","890",9.79136472351,0);
    add_edge_graph(&g1,"598","874",1.9314489524,0);
    add_edge_graph(&g1,"599","761",5.9137497206,0);
    add_edge_graph(&g1,"600","634",6.0956146235,0);
    add_edge_graph(&g1,"601","858",6.61019077967,0);
    add_edge_graph(&g1,"601","795",0.498481695905,0);
    add_edge_graph(&g1,"602","795",7.82369314063,0);
    add_edge_graph(&g1,"603","874",5.02723861746,0);
    add_edge_graph(&g1,"603","772",0.470411343208,0);
    add_edge_graph(&g1,"603","965",2.45530399825,0);
    add_edge_graph(&g1,"605","961",4.21607249376,0);
    add_edge_graph(&g1,"605","739",0.780016340358,0);
    add_edge_graph(&g1,"605","810",1.01592310715,0);
    add_edge_graph(&g1,"605","786",3.52447858661,0);
    add_edge_graph(&g1,"605","890",3.03793854224,0);
    add_edge_graph(&g1,"607","874",5.83310596582,0);
    add_edge_graph(&g1,"607","850",3.26379068692,0);
    add_edge_graph(&g1,"607","890",6.65744924622,0);
    add_edge_graph(&g1,"608","780",6.28464835672,0);
    add_edge_graph(&g1,"613","874",8.14657007925,0);
    add_edge_graph(&g1,"613","890",9.36304939528,0);
    add_edge_graph(&g1,"615","937",8.31304239358,0);
    add_edge_graph(&g1,"615","824",4.16481929983,0);
    add_edge_graph(&g1,"616","890",5.2144595516,0);
    add_edge_graph(&g1,"618","938",4.11437791224,0);
    add_edge_graph(&g1,"621","832",0.310130242427,0);
    add_edge_graph(&g1,"625","936",9.26809258274,0);
    add_edge_graph(&g1,"625","874",3.58824962185,0);
    add_edge_graph(&g1,"625","717",2.75203132525,0);
    add_edge_graph(&g1,"625","941",0.708037797243,0);
    add_edge_graph(&g1,"625","950",6.98364706829,0);
    add_edge_graph(&g1,"625","890",1.56236993291,0);
    add_edge_graph(&g1,"626","786",0.100897004171,0);
    add_edge_graph(&g1,"628","898",8.76887635345,0);
    add_edge_graph(&g1,"628","886",3.85694530385,0);
    add_edge_graph(&g1,"628","744",0.0625030331763,0);
    add_edge_graph(&g1,"628","856",3.44857900018,0);
    add_edge_graph(&g1,"628","888",8.60152679872,0);
    add_edge_graph(&g1,"629","805",6.36023525262,0);
    add_edge_graph(&g1,"631","904",1.0382749532,0);
    add_edge_graph(&g1,"634","890",5.40922209391,0);
    add_edge_graph(&g1,"635","890",0.250251865539,0);
    add_edge_graph(&g1,"637","786",9.89793058754,0);
    add_edge_graph(&g1,"639","874",4.40990413262,0);
    add_edge_graph(&g1,"642","874",9.64185105088,0);
    add_edge_graph(&g1,"642","888",4.86566336003,0);
    add_edge_graph(&g1,"644","942",8.94638043439,0);
    add_edge_graph(&g1,"645","658",7.40297350767,0);
    add_edge_graph(&g1,"645","874",1.34807081012,0);
    add_edge_graph(&g1,"646","874",9.95115998464,0);
    add_edge_graph(&g1,"647","940",2.06315550341,0);
    add_edge_graph(&g1,"648","786",9.31203265472,0);
    add_edge_graph(&g1,"649","855",2.28555526808,0);
    add_edge_graph(&g1,"650","890",5.11529748179,0);
    add_edge_graph(&g1,"650","786",1.39514996547,0);
    add_edge_graph(&g1,"653","890",2.58911148176,0);
    add_edge_graph(&g1,"658","874",4.96109234713,0);
    add_edge_graph(&g1,"658","890",4.81563143455,0);
    add_edge_graph(&g1,"658","828",2.30362301836,0);
    add_edge_graph(&g1,"660","898",6.16725389862,0);
    add_edge_graph(&g1,"660","772",8.95557210304,0);
    add_edge_graph(&g1,"660","795",1.92153353479,0);
    add_edge_graph(&g1,"661","899",5.0474038261,0);
    add_edge_graph(&g1,"663","937",0.0289817243018,0);
    add_edge_graph(&g1,"664","870",1.59031183654,0);
    add_edge_graph(&g1,"666","874",2.39165210719,0);
    add_edge_graph(&g1,"666","733",5.10142661924,0);
    add_edge_graph(&g1,"667","785",9.73911309075,0);
    add_edge_graph(&g1,"667","888",3.1911012684,0);
    add_edge_graph(&g1,"667","916",5.09572814728,0);
    add_edge_graph(&g1,"668","890",8.08740598533,0);
    add_edge_graph(&g1,"671","840",2.63570000363,0);
    add_edge_graph(&g1,"671","874",3.81431139518,0);
    add_edge_graph(&g1,"671","860",3.5859008171,0);
    add_edge_graph(&g1,"675","844",0.954186981913,0);
    add_edge_graph(&g1,"675","780",5.14143304519,0);
    add_edge_graph(&g1,"675","754",6.43775738406,0);
    add_edge_graph(&g1,"675","888",4.74703464716,0);
    add_edge_graph(&g1,"675","890",1.60953573063,0);
    add_edge_graph(&g1,"676","846",0.852983929547,0);
    add_edge_graph(&g1,"679","890",3.2473843357,0);
    add_edge_graph(&g1,"681","890",3.13369633305,0);
    add_edge_graph(&g1,"682","855",2.97599709693,0);
    add_edge_graph(&g1,"683","888",5.21024187586,0);
    add_edge_graph(&g1,"683","874",1.93920941813,0);
    add_edge_graph(&g1,"683","801",6.91847512539,0);
    add_edge_graph(&g1,"683","743",9.76771593635,0);
    add_edge_graph(&g1,"684","874",5.15325173297,0);
    add_edge_graph(&g1,"685","742",8.39579949859,0);
    add_edge_graph(&g1,"685","874",7.86771945497,0);
    add_edge_graph(&g1,"685","780",7.85919182746,0);
    add_edge_graph(&g1,"685","821",2.58658748557,0);
    add_edge_graph(&g1,"685","888",7.84652472487,0);
    add_edge_graph(&g1,"685","890",9.78935966091,0);
    add_edge_graph(&g1,"686","715",3.18360205634,0);
    add_edge_graph(&g1,"686","972",9.30736683317,0);
    add_edge_graph(&g1,"686","789",9.94096335364,0);
    add_edge_graph(&g1,"686","890",8.94604578817,0);
    add_edge_graph(&g1,"687","795",9.27390867561,0);
    add_edge_graph(&g1,"688","828",9.61458051366,0);
    add_edge_graph(&g1,"689","797",3.99416334304,0);
    add_edge_graph(&g1,"689","968",9.86846557736,0);
    add_edge_graph(&g1,"689","748",3.53279588208,0);
    add_edge_graph(&g1,"689","786",1.09832305728,0);
    add_edge_graph(&g1,"689","899",6.9518015239,0);
    add_edge_graph(&g1,"689","916",6.49911099839,0);
    add_edge_graph(&g1,"689","890",9.32652435607,0);
    add_edge_graph(&g1,"690","890",1.82533534303,0);
    add_edge_graph(&g1,"691","984",5.33693253905,0);
    add_edge_graph(&g1,"691","890",0.88970457636,0);
    add_edge_graph(&g1,"693","874",0.953590301931,0);
    add_edge_graph(&g1,"695","795",3.89148440832,0);
    add_edge_graph(&g1,"696","853",7.82032907138,0);
    add_edge_graph(&g1,"697","780",9.41596354217,0);
    add_edge_graph(&g1,"697","874",7.39256188471,0);
    add_edge_graph(&g1,"699","888",0.436654559566,0);
    add_edge_graph(&g1,"701","874",8.65877273009,0);
    add_edge_graph(&g1,"701","778",8.18662282404,0);
    add_edge_graph(&g1,"703","876",9.4861860395,0);
    add_edge_graph(&g1,"704","961",3.53682881223,0);
    add_edge_graph(&g1,"704","810",8.28686557725,0);
    add_edge_graph(&g1,"706","749",7.26068696379,0);
    add_edge_graph(&g1,"707","994",2.12989941894,0);
    add_edge_graph(&g1,"707","786",8.89249455424,0);
    add_edge_graph(&g1,"707","890",0.664742277953,0);
    add_edge_graph(&g1,"707","874",8.51082938327,0);
    add_edge_graph(&g1,"708","824",7.88779463755,0);
    add_edge_graph(&g1,"708","890",9.86214384048,0);
    add_edge_graph(&g1,"709","874",6.61165451607,0);
    add_edge_graph(&g1,"710","888",3.8376273383,0);
    add_edge_graph(&g1,"718","916",3.72176739364,0);
    add_edge_graph(&g1,"718","786",4.82778877167,0);
    add_edge_graph(&g1,"719","828",8.64029344473,0);
    add_edge_graph(&g1,"721","888",5.16982919842,0);
    add_edge_graph(&g1,"722","786",4.88208648134,0);
    add_edge_graph(&g1,"724","824",4.36869284628,0);
    add_edge_graph(&g1,"724","874",9.64770407165,0);
    add_edge_graph(&g1,"725","888",1.46372854481,0);
    add_edge_graph(&g1,"726","874",9.05242448499,0);
    add_edge_graph(&g1,"728","888",2.60922533257,0);
    add_edge_graph(&g1,"731","890",0.107124197649,0);
    add_edge_graph(&g1,"732","783",9.1705975019,0);
    add_edge_graph(&g1,"733","963",4.61570130259,0);
    add_edge_graph(&g1,"735","824",6.46350891366,0);
    add_edge_graph(&g1,"737","890",7.36629089257,0);
    add_edge_graph(&g1,"739","874",0.574114956363,0);
    add_edge_graph(&g1,"739","824",4.30361069383,0);
    add_edge_graph(&g1,"739","888",1.85262854177,0);
    add_edge_graph(&g1,"739","954",7.63278639221,0);
    add_edge_graph(&g1,"741","874",4.57723707224,0);
    add_edge_graph(&g1,"743","937",1.10262268291,0);
    add_edge_graph(&g1,"743","879",6.3907430201,0);
    add_edge_graph(&g1,"744","855",2.60065389684,0);
    add_edge_graph(&g1,"745","870",1.55928290213,0);
    add_edge_graph(&g1,"745","786",4.28075349419,0);
    add_edge_graph(&g1,"745","875",2.27088826611,0);
    add_edge_graph(&g1,"745","761",6.95386424102,0);
    add_edge_graph(&g1,"745","890",0.593561325304,0);
    add_edge_graph(&g1,"748","781",9.6229329224,0);
    add_edge_graph(&g1,"750","890",7.44828521583,0);
    add_edge_graph(&g1,"752","888",6.64220461881,0);
    add_edge_graph(&g1,"756","874",7.88124842096,0);
    add_edge_graph(&g1,"757","888",3.02952035359,0);
    add_edge_graph(&g1,"758","800",0.556697513402,0);
    add_edge_graph(&g1,"759","890",2.29382493611,0);
    add_edge_graph(&g1,"759","787",3.07404422258,0);
    add_edge_graph(&g1,"760","858",3.69057804834,0);
    add_edge_graph(&g1,"761","786",9.90539800917,0);
    add_edge_graph(&g1,"761","968",4.9673774267,0);
    add_edge_graph(&g1,"761","874",1.08855625598,0);
    add_edge_graph(&g1,"761","890",8.11834499235,0);
    add_edge_graph(&g1,"762","890",5.11883698504,0);
    add_edge_graph(&g1,"762","874",1.17871844711,0);
    add_edge_graph(&g1,"763","786",0.617227813369,0);
    add_edge_graph(&g1,"765","858",2.14060648088,0);
    add_edge_graph(&g1,"766","874",7.24944796864,0);
    add_edge_graph(&g1,"768","874",1.30942149484,0);
    add_edge_graph(&g1,"772","996",8.38050903901,0);
    add_edge_graph(&g1,"772","980",4.95685524684,0);
    add_edge_graph(&g1,"772","874",1.79492483803,0);
    add_edge_graph(&g1,"774","890",6.89177502262,0);
    add_edge_graph(&g1,"774","810",2.01616589382,0);
    add_edge_graph(&g1,"775","853",4.02872097692,0);
    add_edge_graph(&g1,"776","874",6.40658541033,0);
    add_edge_graph(&g1,"778","874",2.49348258342,0);
    add_edge_graph(&g1,"779","950",3.07380665241,0);
    add_edge_graph(&g1,"780","845",8.66070233313,0);
    add_edge_graph(&g1,"780","874",8.78386048827,0);
    add_edge_graph(&g1,"780","888",8.22015222981,0);
    add_edge_graph(&g1,"786","953",4.58628386522,0);
    add_edge_graph(&g1,"786","795",6.70342765248,0);
    add_edge_graph(&g1,"786","827",2.13408482922,0);
    add_edge_graph(&g1,"786","832",2.52063740996,0);
    add_edge_graph(&g1,"786","838",6.2496830819,0);
    add_edge_graph(&g1,"786","846",3.65149625051,0);
    add_edge_graph(&g1,"786","874",3.37310700904,0);
    add_edge_graph(&g1,"786","876",1.16913289587,0);
    add_edge_graph(&g1,"786","888",0.832551659219,0);
    add_edge_graph(&g1,"786","890",9.16513172634,0);
    add_edge_graph(&g1,"786","954",6.93641056623,0);
    add_edge_graph(&g1,"786","986",5.65481913466,0);
    add_edge_graph(&g1,"790","890",3.05967847275,0);
    add_edge_graph(&g1,"793","874",1.11233310642,0);
    add_edge_graph(&g1,"793","902",6.27145934498,0);
    add_edge_graph(&g1,"794","976",3.58261375842,0);
    add_edge_graph(&g1,"795","930",8.71676862297,0);
    add_edge_graph(&g1,"795","954",7.76408487315,0);
    add_edge_graph(&g1,"795","966",5.9350963845,0);
    add_edge_graph(&g1,"795","874",5.14284557341,0);
    add_edge_graph(&g1,"795","890",2.71269549581,0);
    add_edge_graph(&g1,"795","892",4.49553925931,0);
    add_edge_graph(&g1,"797","954",2.85692199041,0);
    add_edge_graph(&g1,"797","890",9.75492203677,0);
    add_edge_graph(&g1,"800","888",6.8498647327,0);
    add_edge_graph(&g1,"800","989",6.4333622241,0);
    add_edge_graph(&g1,"802","968",2.99959522347,0);
    add_edge_graph(&g1,"806","874",8.37441191143,0);
    add_edge_graph(&g1,"807","890",9.49023465666,0);
    add_edge_graph(&g1,"809","890",8.52980985577,0);
    add_edge_graph(&g1,"810","914",8.97821185584,0);
    add_edge_graph(&g1,"810","888",9.22457553128,0);
    add_edge_graph(&g1,"811","882",1.76611880932,0);
    add_edge_graph(&g1,"812","968",7.25982524604,0);
    add_edge_graph(&g1,"812","874",4.97094878459,0);
    add_edge_graph(&g1,"814","890",2.22420344815,0);
    add_edge_graph(&g1,"814","855",4.12223506934,0);
    add_edge_graph(&g1,"815","874",5.08561545938,0);
    add_edge_graph(&g1,"816","981",0.379761150512,0);
    add_edge_graph(&g1,"817","888",4.21786714895,0);
    add_edge_graph(&g1,"818","858",7.32519097451,0);
    add_edge_graph(&g1,"820","874",4.89575826937,0);
    add_edge_graph(&g1,"822","888",9.91883231263,0);
    add_edge_graph(&g1,"822","879",4.98379440147,0);
    add_edge_graph(&g1,"824","924",2.84276259106,0);
    add_edge_graph(&g1,"824","961",7.10569621802,0);
    add_edge_graph(&g1,"824","888",3.80431387061,0);
    add_edge_graph(&g1,"824","855",7.22495897956,0);
    add_edge_graph(&g1,"824","847",0.492663595157,0);
    add_edge_graph(&g1,"824","990",8.44782397055,0);
    add_edge_graph(&g1,"824","874",0.851541619186,0);
    add_edge_graph(&g1,"824","890",3.80581206979,0);
    add_edge_graph(&g1,"828","874",5.94076053789,0);
    add_edge_graph(&g1,"828","890",5.14860104155,0);
    add_edge_graph(&g1,"829","902",1.28131983412,0);
    add_edge_graph(&g1,"832","941",2.58681616734,0);
    add_edge_graph(&g1,"832","888",8.90883494062,0);
    add_edge_graph(&g1,"835","968",7.22081488717,0);
    add_edge_graph(&g1,"835","888",2.51439037544,0);
    add_edge_graph(&g1,"835","874",2.07046659341,0);
    add_edge_graph(&g1,"840","874",9.67891168336,0);
    add_edge_graph(&g1,"840","854",8.98585791919,0);
    add_edge_graph(&g1,"842","874",4.13206078598,0);
    add_edge_graph(&g1,"843","888",7.43182038067,0);
    add_edge_graph(&g1,"845","890",4.10358715272,0);
    add_edge_graph(&g1,"849","890",8.62059880586,0);
    add_edge_graph(&g1,"850","888",5.27397103105,0);
    add_edge_graph(&g1,"850","890",2.21937953501,0);
    add_edge_graph(&g1,"850","874",9.86543659868,0);
    add_edge_graph(&g1,"852","874",2.69890204879,0);
    add_edge_graph(&g1,"852","890",6.02684852349,0);
    add_edge_graph(&g1,"853","888",3.54767047064,0);
    add_edge_graph(&g1,"854","888",5.21377784691,0);
    add_edge_graph(&g1,"855","918",9.9958605274,0);
    add_edge_graph(&g1,"855","959",8.45868019629,0);
    add_edge_graph(&g1,"855","997",4.10466060356,0);
    add_edge_graph(&g1,"855","874",4.68895116897,0);
    add_edge_graph(&g1,"855","888",3.55272199443,0);
    add_edge_graph(&g1,"855","890",2.1372117038,0);
    add_edge_graph(&g1,"855","938",4.71022785212,0);
    add_edge_graph(&g1,"858","888",8.40165656817,0);
    add_edge_graph(&g1,"858","890",9.95975289003,0);
    add_edge_graph(&g1,"860","874",0.0926680981539,0);
    add_edge_graph(&g1,"864","874",7.10359781995,0);
    add_edge_graph(&g1,"866","888",0.993615738303,0);
    add_edge_graph(&g1,"866","890",1.19362125035,0);
    add_edge_graph(&g1,"866","874",5.24072260787,0);
    add_edge_graph(&g1,"867","890",2.41384943398,0);
    add_edge_graph(&g1,"868","874",0.113043948441,0);
    add_edge_graph(&g1,"869","874",9.38700305742,0);
    add_edge_graph(&g1,"870","890",3.18200126763,0);
    add_edge_graph(&g1,"871","888",6.91866735664,0);
    add_edge_graph(&g1,"871","880",5.5732421343,0);
    add_edge_graph(&g1,"871","890",1.57522125293,0);
    add_edge_graph(&g1,"873","874",8.89466783206,0);
    add_edge_graph(&g1,"874","890",2.5453312541,0);
    add_edge_graph(&g1,"874","893",2.10615706041,0);
    add_edge_graph(&g1,"874","961",4.27623097376,0);
    add_edge_graph(&g1,"874","879",7.82523832812,0);
    add_edge_graph(&g1,"874","915",6.86838390188,0);
    add_edge_graph(&g1,"874","884",1.71672332017,0);
    add_edge_graph(&g1,"874","888",6.59556392367,0);
    add_edge_graph(&g1,"874","899",8.46827067675,0);
    add_edge_graph(&g1,"874","903",9.4451119061,0);
    add_edge_graph(&g1,"874","916",5.70269404427,0);
    add_edge_graph(&g1,"874","919",4.45026480831,0);
    add_edge_graph(&g1,"874","933",7.6545589017,0);
    add_edge_graph(&g1,"874","938",8.93355836712,0);
    add_edge_graph(&g1,"874","940",2.57389680985,0);
    add_edge_graph(&g1,"874","947",5.01326307247,0);
    add_edge_graph(&g1,"874","950",1.05799320668,0);
    add_edge_graph(&g1,"874","963",5.34418421399,0);
    add_edge_graph(&g1,"874","968",3.833857436,0);
    add_edge_graph(&g1,"874","935",9.90638880604,0);
    add_edge_graph(&g1,"874","937",5.24552688021,0);
    add_edge_graph(&g1,"881","890",5.65741715983,0);
    add_edge_graph(&g1,"882","890",5.53523764932,0);
    add_edge_graph(&g1,"885","968",6.72895883526,0);
    add_edge_graph(&g1,"888","916",4.75823790446,0);
    add_edge_graph(&g1,"888","890",4.88184842862,0);
    add_edge_graph(&g1,"888","898",5.80776790951,0);
    add_edge_graph(&g1,"888","900",1.01153316264,0);
    add_edge_graph(&g1,"888","903",1.85611099122,0);
    add_edge_graph(&g1,"888","919",9.09798369673,0);
    add_edge_graph(&g1,"888","933",5.32480605657,0);
    add_edge_graph(&g1,"888","940",0.7858588663,0);
    add_edge_graph(&g1,"888","950",6.38193433861,0);
    add_edge_graph(&g1,"888","957",3.13800565469,0);
    add_edge_graph(&g1,"888","962",0.911524420849,0);
    add_edge_graph(&g1,"888","963",5.95489079561,0);
    add_edge_graph(&g1,"888","973",4.3876741032,0);
    add_edge_graph(&g1,"888","975",6.7815721418,0);
    add_edge_graph(&g1,"888","977",5.74108823015,0);
    add_edge_graph(&g1,"888","997",7.84447067762,0);
    add_edge_graph(&g1,"888","999",0.614509448933,0);
    add_edge_graph(&g1,"890","945",9.46685883747,0);
    add_edge_graph(&g1,"890","898",7.20789477345,0);
    add_edge_graph(&g1,"890","916",6.03134202794,0);
    add_edge_graph(&g1,"890","903",0.0100048885989,0);
    add_edge_graph(&g1,"890","932",7.0414966756,0);
    add_edge_graph(&g1,"890","934",0.987043784212,0);
    add_edge_graph(&g1,"890","937",4.6930263143,0);
    add_edge_graph(&g1,"890","940",7.16588615105,0);
    add_edge_graph(&g1,"890","986",4.69392839638,0);
    add_edge_graph(&g1,"890","946",8.78682238076,0);
    add_edge_graph(&g1,"890","950",6.39578737815,0);
    add_edge_graph(&g1,"890","960",3.884585954,0);
    add_edge_graph(&g1,"890","998",2.10373359252,0);
    add_edge_graph(&g1,"898","992",7.96079313639,0);
    add_edge_graph(&g1,"898","961",2.70122125207,0);
    add_edge_graph(&g1,"899","984",9.19157942651,0);
    add_edge_graph(&g1,"904","978",4.93399198524,0);
    add_edge_graph(&g1,"906","915",1.08810626336,0);
    add_edge_graph(&g1,"916","970",8.0600917674,0);
    add_edge_graph(&g1,"923","961",0.881081651573,0);
    add_edge_graph(&g1,"929","951",6.62061835136,0);
    add_edge_graph(&g1,"930","970",6.01230348366,0);
    add_edge_graph(&g1,"936","937",2.2035514574,0);
    add_edge_graph(&g1,"939","954",7.26301290805,0);
    add_edge_graph(&g1,"940","994",6.22002237053,0);
    add_edge_graph(&g1,"944","954",9.226654919,0);
    add_edge_graph(&g1,"950","961",3.59295649502,0);
    add_edge_graph(&g1,"950","967",0.66838757152,0);
    add_edge_graph(&g1,"961","968",0.428927174717,0);
    add_edge_graph(&g1,"963","980",4.63289851941,0);
    
    
    
    
    
    
    
    
    struct node_list *  nl;
    double * bh=betweeness_brandes(&g1,true,0);
    double * bh_c=betwenness_heuristic(&g1,false);
    //double * bh_c2=betwenness_heuristic(&g1,true);
    
    
    for(nl=g1.nodes.head;nl!=0;nl=nl->next){
        struct node_graph * ng=(struct node_graph*)nl->content;
        
        //printf("%s:\t%f \t%f\t%d\t%1.50f\n",
        //printf("%s:\t%1.50f \t%1.50f\t%d\t%1.50f\n",
        printf("%s:\t%f \t%f\t%d\t%f\n",
                ng->name,
                bh[ng->node_graph_id], 
                bh_c[ng->node_graph_id],
                bh[ng->node_graph_id]==bh_c[ng->node_graph_id],
                -bh[ng->node_graph_id]+bh_c[ng->node_graph_id]);
        if((-bh[ng->node_graph_id]+bh_c[ng->node_graph_id])!=0){
            //printf("%1.50f\n",
            //printf("%s:\t%1.50f \t%1.50f\t%d\t%1.50f\n",
            //       -bh[ng->node_graph_id]+bh_c[ng->node_graph_id]);
        }
        
    }
    free(bh);
    free(bh_c);
    //free(bh_c2);
    free_graph(&g1);
    return 0;
}

*/