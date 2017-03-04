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

static inline double round_decimal(double d){
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
    float standard_deviation_bic=-1;
    float standard_deviation_edge=0;

    if(stop_computing_if_unchanged){
        int edge_num=0;
        struct node_list * nl=g->nodes.head;
        for(;nl!=0;nl=nl->next){
            struct node_graph * ng=(struct node_graph *)nl->content;
            struct node_list * nl2=ng->neighbours.head;
           for(;nl2!=0;nl2=nl2->next){
                struct edge_graph * eg=(struct edge_graph *)nl2->content;
                standard_deviation_edge+=eg->value;
                edge_num++;
            }
        }
        standard_deviation_edge/=edge_num;
        //if we rely on old values when network
        //is not changed
        char ** old_names=0;
        double *  old_ret_val=is_network_changed(connected_components_subgraphs,g->nodes.size,
                &biconnected_component_num,&standard_deviation_bic,&standard_deviation_edge,&result_size,&old_names);
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
        // struct sub_graph * sg=(struct sub_graph *)dequeue_list(connected_components_subgraphs);
        if(cc_num>1||use_heu_on_single_biconnected){
            struct node_list * subgraph_iterator=connected_components_subgraphs->head;
            for(;subgraph_iterator!=0;subgraph_iterator=subgraph_iterator->next){
                struct sub_graph * sg=(struct sub_graph *)subgraph_iterator->content;
                compute_heuristic_wo_scale(g,&(sg->connected_components),
                        is_articulation_point,ret_val,connected_component_indexes,
                        sg->size,connected_component_index++);
            }
        }else {
            clear_list(connected_components_subgraphs);
            free(connected_components_subgraphs);
            free(is_articulation_point);
            free(connected_component_indexes);
            free(ret_val);
            return betweeness_brandes(g,true,0);
        }
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
        write_file(biconnected_component_num,standard_deviation_bic,standard_deviation_edge,node_num,ret_val,&g->nodes);
    }


    clear_list(connected_components_subgraphs);
    free(connected_components_subgraphs);
    free(is_articulation_point);
    free(connected_component_indexes);


    return ret_val;
}
