#include "graph.h"

void init_node_graph(struct node_graph * n,const char * name, int node_graph_id);
void init_edge_graph(struct edge_graph * e);
void init_edge_graph_params(struct edge_graph * e,struct node_graph * to,double value);

/**
 * Constructor of a graph. Given a graph, it initialities its parameter.
 * The graph is implemented as a list of nodes and neighbours, so our 
 * computation load is in insertion, not in all successive operations.
 * 
 * @param g An uninitialized graph
 */
void init_graph(struct graph * g){
    init_list(&(g->nodes));
}

/**
 * Adds node to graph given its id, @name.
 * It performs only the numbering, i.e. the node is a new one and its id is
 * the size of the graph before its insertion.
 * Node is returned to use it for edge creation.
 * This is an helper function, you should use add_edge_graph
 * @param g A graph
 * @param name The name of the node that will be added
 * @return The created and added node
 */
struct node_graph * add_node_graph(struct graph * g, const char * name){//uniqueness check not performed
    struct node_graph * n=(struct node_graph*)malloc(sizeof(struct node_graph));
    init_node_graph(n,name,g->nodes.size);
    enqueue_list(&(g->nodes),(void*)n);
    return n;
}

/**
 * Adds edge to given graph.
 * This is the proper way to create a graph, given the names of an edge 
 * vertexes. It does not matter whether nodes are already present. If not, they
 * are created.
 * The complexity of this insertion, which gives from granted that only a new
 * edge are added, is O(|N|+|E|), i.e. linear to the number of nodes and edges.
 * Given that medium degree of real graph for this project is 1.2, we have a 
 * good average complexity.
 * 
 * @param g A graph
 * @param name_from The vertex name in which the edge originates
 * @param name_to The vertex name in which the edge terminates
 * @param value The edge weight
 * @param directed Whether the edge is directed or not
 */
void add_edge_graph(struct graph * g, const char * name_from, const char * name_to, double value, bool directed){
    struct node_graph *from=0,*to=0,*current =0;
    struct node_list * n=g->nodes.head;
    while(n!=0 && (from==0 || to==0)){ //if there are no more nodes or we have found both edge ends
        current=(struct node_graph *)n->content;
        if(from==0 && strcmp(current->name,name_from)==0){
            from=current;
        }
        if(to==0 && strcmp(current->name,name_to)==0){
            to=current;
        }
        n=n->next;
    }
    if(from==0){
        from=add_node_graph(g,name_from);
        if(strcmp(name_from,name_to)==0)
            to=from;
    }     
    if(to==0){
        to=add_node_graph(g,name_to);
    }
    if(from!=0 && to!=0){
        struct edge_graph * e=(struct edge_graph*)malloc(sizeof(struct edge_graph));
        init_edge_graph_params(e,to,value);
        enqueue_list(&(from->neighbours),(void*)e);
        if(!directed){ 
            struct edge_graph * e_r=(struct edge_graph*)malloc(sizeof(struct edge_graph));
            init_edge_graph_params(e_r,from,value);
            enqueue_list(&(to->neighbours),(void*)e_r);
        }
    }
}

/**
 * Adds edge to given graph.
 * This function is used to create graph that represents subgraphs. Therefore 
 * nodes will have a different id.
 * 
 * @param g A graph
 * @param name_from The vertex name in which the edge originates
 * @param name_to The vertex name in which the edge terminates
 * @param value The edge weight
 * @param directed Whether the edge is directed or not
 * @param nodefrom An integer pointer used to return the id of @name_from node
 * @param nodeto An integer pointer used to return the id of @name_to node
 */
void add_edge_graph_return_node_indexes(struct graph * g, const char * name_from, const char * name_to, double value, bool directed,int  * nodefrom, int * nodeto){
    struct node_graph *from=0,*to=0,*current =0;
    struct node_list * n=g->nodes.head;
    while(n!=0 && (from==0 || to==0)){ //if there are no more nodes or we have found both edge ends
        current=(struct node_graph *)n->content;
        if(from==0 && strcmp(current->name,name_from)==0){
            from=current;
        }
        if(to==0 && strcmp(current->name,name_to)==0){
            to=current;
        }
        n=n->next;
    }
    if(from==0){
        from=add_node_graph(g,name_from);
        if(strcmp(name_from,name_to)==0)
            to=from;
    }     
    if(to==0){
        to=add_node_graph(g,name_to);
    }
    if(from!=0 && to!=0){
        if(nodefrom!=0)
            (*nodefrom)=from->node_graph_id;
        if(nodeto!=0)
            (*nodeto)=to->node_graph_id;
        struct edge_graph * e=(struct edge_graph*)malloc(sizeof(struct edge_graph));
        init_edge_graph_params(e,to,value);
        enqueue_list(&(from->neighbours),(void*)e);
        if(!directed){ 
            struct edge_graph * e_r=(struct edge_graph*)malloc(sizeof(struct edge_graph));
            init_edge_graph_params(e_r,from,value);
            enqueue_list(&(to->neighbours),(void*)e_r);
        }
    } 
    
}

/**
 * Helper function. Given a graph, it prints it as a list of nodes and adjacency
 * list.
 * 
 * @param g A graph
 */
void print_graph(struct graph * g){
    struct node_list * nq=g->nodes.head;
    while(nq!=0){
        struct node_graph * ng=(struct node_graph*)nq->content;
        struct node_list * nqi=ng->neighbours.head;
        printf("%s (%d) [",ng->name,ng->node_graph_id);
        while(nqi!=0){
            struct edge_graph * eg=(struct edge_graph*)nqi->content;
            printf(" (%s , %f) ",eg->to->name,eg->value);
            nqi=nqi->next;
        } 
        printf("]\n");
        nq=nq->next;
    }
}

/**
 * Node initializer. 
 * Given a node, it initializes its parameter.
 * Helper function
 *  
 * @param n A node
 * @param name The name that identifies a node. Must be unique
 * @param node_graph_id The numeric id of the new node.
 */
void init_node_graph(struct node_graph * n,const char * name,int node_graph_id){
    n->name=strdup((const char *)name);
    init_list(&(n->neighbours));
    n->node_graph_id=node_graph_id;
}

/**
 * Edge initializer. 
 * Given a edge, it initializes its parameter.
 * Helper function
 *  
 * @param e An edge
 */
void init_edge_graph(struct edge_graph * e){
    e->to=0;
    e->value=0;
}

/**
 * Edge initializer. 
 * Given a edge, it initializes its parameter with given ones.
 * Helper function
 *  
 * @param e An edge
 * @param to The node that is the vertex of the edge
 * @param value The value of the link
 */
void init_edge_graph_params(struct edge_graph * e,struct node_graph * to,double value){
    e->to=to;
    e->value=value;
}

/**
 *  Empties the graph deallocating its memory. The graph can be either in
 * static or dynamic memory.
 *  
 * @param g A graph to be deallocated.
 * 
 */
void free_graph(struct graph * g){
    struct node_list * nq=g->nodes.head;
    while(nq!=0){
        struct node_list * nq_tmp=nq;
        struct node_graph * ng=(struct node_graph*)nq->content;
        struct node_list * eq=(struct node_list*)ng->neighbours.head;
        struct node_list * eq_tmp;
        while(eq!=0){
            eq_tmp=eq;
            struct edge_graph *e=(struct edge_graph*)eq->content;
            eq=eq->next;
            free(eq_tmp);
            free(e);
        }
        nq=nq->next;
        free(ng->name);
        free(ng);
        free(nq_tmp);
    }
}