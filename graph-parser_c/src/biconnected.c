#include "biconnected.h"

//pseudocode from https://en.wikipedia.org/wiki/Tarjan's_strongly_connected_components_algorithm




void strongconnect(struct node_graph* v,int * current_index, struct list * s,
        int * bcc_id,struct list* connected_components, int * index,
        int * low_link,bool * on_stack);  
void DFS_iter(struct node_graph* u,int * current_index,struct list * s,
        int *bcc_id,struct list* connected_components,
        struct node_graph ** caller, struct node_list ** iterator,
        int * index,int * low_link,bool * on_stack);
void DFS_visit(struct node_graph * u,struct list *s,int * d,int * low,
        bool * visited,struct node_graph ** parent,int * count, bool * added, 
        bool * is_articulation_point,int node_num,
        struct sub_graph * sg, int * component_indexes, 
        int component_index);

/**
 * Returns the min of two numbers. It is inline, so it simplyifies code withouth
 * degrading performance.
 * 
 * @param a An integer
 * @param b An integer
 * @return the minimum value of @a and @b. 
 */
static inline int min(int a, int b){
    if(a<b)
        return a;
    return b;
}

/**
 * Given a directed graph, it returns the biconnected components. 
 * The search is recursive
 * Not employed for now.
 * 
 * @param g A graph
 * @return A list of biconnected components
 */
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

/**
 * Given a node, it returns all the biconnected components in the connected 
 * graph. 
 *  
 * Based on https://en.wikipedia.org/wiki/Tarjan's_strongly_connected_components_algorithm
 * 
 * @param v A node
 * @param current_index current index value, used for detecting component
 * @param s list working as a stack of nodes
 * @param int pointer to an integer representing the bcc_id id of the 
 * biconnected component
 * @param connected_components list of connected_components list of 
 * biconnected component
 * @param index int array, actual index value for each node
 * @param low_link int array, actual low_link value for each node
 * @param on_stack bool array, tells whether a node is on the stack s
 */
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

/**
 * Given a directed graph, it returns the biconnected components. 
 * The search is iterative
 * Not employed for now.
 * 
 * @param g A graph
 * @return A list of biconnected components
 */
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
/**
 * Given a node, it returns all the biconnected components in the connected 
 * graph. 
 * 
 * 
 * @param u A node
 * @param current_index  current index value, used for detecting component
 * @param s list working as a stack of nodes
 * @param bcc_id id of the current biconnected components
 * @param connected_components list of connected_components list of 
 * biconnected component
 * @param caller pointer to caller node, used to mimic recursive behaviour
 * @param iterator pointer to current node, used to mimic recursive behaviour
 * @param index int array, actual index value for each node
 * @param low_link int array, actual low_link value for each node
 * @param on_stack bool array, tells whether a node is on the stack s
 */
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

/**
 * Subgraph initializers. 
 * It creates and initializes an empty biconnected component list
 * and set the connected component size to 0.
 * 
 * @return The newly created subgrtaph struct
 */
struct sub_graph * init_sub_graph(){
    struct sub_graph * sg=( struct sub_graph * )malloc(sizeof( struct sub_graph ));
    init_list(&sg->connected_components);
    sg->size=0;
    return sg;
}

//From http://www.cs.umd.edu/class/fall2005/cmsc451/biconcomps.pdf
/**
 * Given an undirected graph, it returns a list of subgraph, each one collects 
 * a list of biconnected components and the node number in it.
 * The search is recursive
 * 
 * @param g A graph
 * @param is_articulation_point An array representing whether a node is an
 * articulation point. Used as return value
 * @param component_indexes An integer array telling to which connected component a
 * node belongs to.
 * @return a list of pair, one for each connected component, of biconnected 
 * components and node number in that connected component
 */
struct list*  tarjan_rec_undir(struct graph * g, bool * is_articulation_point,
        int * component_indexes){
    struct list * connected_components_subgraph=( struct list * )malloc(sizeof( struct list ));
    init_list(connected_components_subgraph);
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
    int component_index=0;
    for(n =g->nodes.head;n!=0;n=n->next){
        struct node_graph* ng=(struct node_graph*) n->content;
        if(!visited[i]){
            struct sub_graph * sg=init_sub_graph();
            DFS_visit(ng,&s,d,low,visited,parent,&count,added, is_articulation_point,node_num,sg,component_indexes,component_index);
            if(ng->neighbours.size>1)
                is_articulation_point[ng->node_graph_id]=true;
            component_index++;
            enqueue_list(connected_components_subgraph,sg);
            
        }
        i++;
    }
    free(added);
    free(visited);
    free(parent);
    free(d);
    free(low);
    return connected_components_subgraph;
}

struct edge_repr{
    struct node_graph *from;
    struct node_graph *to;  
    double value;
};
/**
 * Edge representation which collects both source and target vertex of an edge.
 * 
 * @param from Node generating the edge
 * @param to Node in which edge terminates
 * @param value Weight of the edge
 * @return The newly created edge
 */
struct edge_repr * init_edge_repr(struct node_graph * from,struct node_graph * to, double value){
    struct edge_repr * er=(struct edge_repr * )malloc(sizeof(struct edge_repr ));
    er->from=from;
    er->to=to;
    er->value=value;
    return er;
}

//https://www.cs.umd.edu/class/fall2005/cmsc451/biconcomps.pdf
/**
 * This function is similar to the one of directed graph. It performs 
 * recursively a visit to detected articulation point, biconnected components 
 * and connected components indexes.
 * 
 * @param u The current node we are starting from-
 * @param s A list, used as stck of nodes
 * @param d Int array, which are indexes for each array
 * @param low Int array, which are low link for each array
 * @param visited Bool array, whether a node is visited or not
 * @param parent Pointer to node from which we are coming from in our wisit.
 * @param count Current index value
 * @param added number of node added, used for creating biconnected component
 * @param is_articulation_point An array representing whether a node is an
 * articulation point. Used as return value
 * @param node_num Total number of nodes in the graph
 * @param sg Current subgraph
 * @param component_indexes Int array, tell to which connected components nodes 
 * belong.
 * @param component_index Int value that tells current connected components 
 * index.
 */
void DFS_visit(struct node_graph * u,struct list * s,int * d,int * low,bool * visited,
        struct node_graph ** parent,int * count, bool * added, bool * is_articulation_point,
        int node_num,struct sub_graph * sg,
        int * component_indexes, int component_index){
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
                DFS_visit(v,s,d,low,visited,parent,count,added,is_articulation_point,node_num,sg,component_indexes, component_index);
                if(low[v->node_graph_id]>=d[u->node_graph_id]){
                    struct edge_repr * er_i=0;
                    struct list l;
                    init_list(&l);
                    do{
                        er_i=( struct edge_repr *)pop_list(s);
                        added[er_i->from->node_graph_id]=true;
                        added[er_i->to->node_graph_id]=true;
                        if(component_indexes[er_i->from->node_graph_id]==-1){
                            component_indexes[er_i->from->node_graph_id]=component_index;
                            sg->size++;
                        }
                        if(component_indexes[er_i->to->node_graph_id]==-1){
                            component_indexes[er_i->to->node_graph_id]=component_index;
                            sg->size++;
                        }
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
                        cc->cutpoint=0;
                        while(!is_empty_list(&l)){
                            struct edge_repr * er_i=( struct edge_repr *)pop_list(&l);
                            int f=0,t=0;
                            add_edge_graph_return_node_indexes(&(cc->g), er_i->from->name, er_i->to->name, er_i->value,0,&f, &t);
                            cc->mapping[f]= er_i->from->node_graph_id;
                            cc->mapping[t]= er_i->to->node_graph_id;
                            free(er_i);
                        }
                        enqueue_list(&(sg->connected_components),cc);
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

struct cc_edge_stack{
    struct node_graph * grandparent;
    struct node_graph * parent;
    struct node_list * iterator;
};

/**
 * Initializer of a biconnected component edge. Used for memorization in a stack
 * in the iterative version of biconnected components data relatives to an edge.
 * It memorizes the current node, its parent and an iterator to first unvisted
 * neighbour, i.e. the first one.
 * @param grandparent Node from which we visited the current node
 * @param parent  Current node
 * @return Newly created edge stack.
 */
struct cc_edge_stack * init_cc_edge_stack(struct node_graph * grandparent,struct node_graph * parent){
    struct cc_edge_stack * ced=(struct cc_edge_stack * )malloc(sizeof(struct cc_edge_stack ));
    ced->parent=parent;
    ced->grandparent=grandparent;
    ced->iterator=parent->neighbours.head;
    return ced;
}


//https://github.com/networkx/networkx/blob/master/networkx/algorithms/components/biconnected.py#L427
/**
 * Given an undirected graph, it returns a list of subgraph, each one collects 
 * a list of biconnected components and the node number in it.
 * The search is iterative
 * 
 * @param g A graph
 * @param is_articulation_point  An array representing whether a node is an
 * articulation point. Used as return value
 * @param component_indexes An integer array telling to which connected component a
 * node belongs to.
 * @return a list of pair, one for each connected component, of biconnected 
 * components and node number in that connected component
 */
struct list*  tarjan_iter_undir(struct graph * g, bool * is_articulation_point, int * component_indexes){
    struct list * connected_components_subgraph=( struct list * )malloc(sizeof( struct list ));
    init_list(connected_components_subgraph);
    struct sub_graph * sg=init_sub_graph();
    int component_index=0;
    int node_num=g->nodes.size;
    bool * visited=( bool * )malloc(sizeof(bool)*node_num);
    bool * added=( bool * )malloc(sizeof(bool)*node_num);
    int * discovery=( int * )malloc(sizeof(int)*node_num);
    int * low=( int * )malloc(sizeof(int)*node_num);
    int i;
    for(i=0;i<node_num;i++){
        is_articulation_point[i]=false;
        visited[i]=false;
        added[i]=false;
    }
    struct node_list * n;
    for(n =g->nodes.head;n!=0;n=n->next){
        struct node_graph *ng=(struct node_graph *)n->content;
        if(!visited[ng->node_graph_id]){
            visited[ng->node_graph_id]=true;
            int discovery_len=1;
            int root_children=0;
            for(i=0;i<node_num;i++){
                discovery[i]=0;
                low[i]=0;
            }
            struct list edge_stack;
            init_list(&edge_stack);
            struct list s;
            init_list(&s);
            enqueue_list(&s,init_cc_edge_stack(ng,ng));
            while(!is_empty_list(&s)){
                struct cc_edge_stack * ced=(struct cc_edge_stack * )peek_last_list(&s);
                if(ced->iterator!=0){
                    struct edge_graph * edge=(struct  edge_graph*)ced->iterator->content;
                    struct node_graph * child=edge->to;
                    ced->iterator=ced->iterator->next;
                    if(child==ced->grandparent){
                        continue;
                    }
                    if(visited[child->node_graph_id]){
                        if(discovery[child->node_graph_id]<=discovery[ced->parent->node_graph_id]){
                            low[ced->parent->node_graph_id] = min(low[ced->parent->node_graph_id], discovery[child->node_graph_id]);
                            enqueue_list(&edge_stack,init_edge_repr(ced->parent,child,edge->value)); 
                        }
                    }else{
                        low[child->node_graph_id]=discovery_len;
                        discovery[child->node_graph_id]=discovery_len;
                        discovery_len++;
                        visited[child->node_graph_id]=true;
                        enqueue_list(&s,init_cc_edge_stack(ced->parent,child));
                        enqueue_list(&edge_stack,init_edge_repr(ced->parent,child,edge->value));
                    }
                }else{
                    struct cc_edge_stack * ced_to_clear=(struct cc_edge_stack * )pop_list(&s);
                    if(s.size>1){
                        if(low[ced->parent->node_graph_id]>=discovery[ced->grandparent->node_graph_id]){
                            struct edge_repr * er=(struct edge_repr *) peek_last_list(&edge_stack);
                            struct list l;
                            init_list(&l);
                            int added_count=0;
                            while(er!=0){
                                enqueue_list(&l,er);
                                if(component_indexes[er->from->node_graph_id]==-1){
                                    component_indexes[er->from->node_graph_id]=component_index;
                                    sg->size++;
                                }
                                if(component_indexes[er->to->node_graph_id]==-1){
                                    component_indexes[er->to->node_graph_id]=component_index;
                                    sg->size++;
                                }
                                if(!added[er->from->node_graph_id])
                                    added_count++;
                                
                                if(!added[er->to->node_graph_id])
                                    added_count++;
                                
                                added[er->from->node_graph_id]=true;
                                added[er->to->node_graph_id]=true;
                                pop_list(&edge_stack);
                                if((er->from==ced->grandparent && er->to==ced->parent)){
                                    break;
                                }
                                er=(struct edge_repr *) peek_last_list(&edge_stack);
                            }
                            
                            if(!is_empty_list(&l)){
                                struct connected_component * cc=(struct connected_component *)malloc(sizeof(struct connected_component));
                                init_graph(&(cc->g));
                                cc->mapping=(int *)malloc(sizeof(int)*added_count);
                                cc->weights=(int *)malloc(sizeof(int)*added_count);
                                for(i=0;i<added_count;i++){
                                    cc->weights[i]=-1;
                                }
                                cc->cutpoint=0;
                                while(!is_empty_list(&l)){
                                    struct edge_repr * er_i=( struct edge_repr *)pop_list(&l);
                                    added[er_i->from->node_graph_id]=false;
                                    added[er_i->to->node_graph_id]=false;
                                    int f=0,t=0;
                                    add_edge_graph_return_node_indexes(&(cc->g), er_i->from->name, er_i->to->name, er_i->value,0,&f, &t);
                                    cc->mapping[f]= er_i->from->node_graph_id;
                                    cc->mapping[t]= er_i->to->node_graph_id;
                                    free(er_i);
                                }
                                enqueue_list(&(sg->connected_components),cc);
                            }
                            is_articulation_point[ced->grandparent->node_graph_id]=true;
                        } 
                        low[ced->grandparent->node_graph_id]=min(low[ced->parent->node_graph_id],low[ced->grandparent->node_graph_id]);
                    }else if(s.size==1){
                        root_children++;
                        struct node_list * nl= edge_stack.tail;
                        struct edge_repr * er=(struct edge_repr *) nl->content;
                        struct list l;
                        init_list(&l);
                        int added_count=0;
                        while(er!=0){
                            enqueue_list(&l,er);
                            if(component_indexes[er->from->node_graph_id]==-1){
                                component_indexes[er->from->node_graph_id]=component_index;
                                sg->size++;
                            }
                            if(component_indexes[er->to->node_graph_id]==-1){
                                component_indexes[er->to->node_graph_id]=component_index;
                                sg->size++;
                            }
                            if(!added[er->from->node_graph_id])
                                added_count++; 
                            if(!added[er->to->node_graph_id])
                                added_count++;
                            nl=nl->prev;
                            if(er->from==ced->grandparent && er->to==ced->parent)
                                break;
                            if(nl!=0){
                                er=(struct edge_repr *) nl->content;
                            }else{
                                er=0;
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
                            cc->cutpoint=0;
                            while(!is_empty_list(&l)){
                                struct edge_repr * er_i=( struct edge_repr *)pop_list(&l);
                                added[er_i->from->node_graph_id]=false;
                                added[er_i->to->node_graph_id]=false;
                                int f=0,t=0;
                                add_edge_graph_return_node_indexes(&(cc->g), er_i->from->name, er_i->to->name, er_i->value,0,&f, &t);
                                cc->mapping[f]= er_i->from->node_graph_id;
                                cc->mapping[t]= er_i->to->node_graph_id;
                                free(er_i);
                            }
                            enqueue_list(&(sg->connected_components),cc);
                        }
                    }
                    free(ced_to_clear);
                }
            }
            if(root_children>1){
                is_articulation_point[ng->node_graph_id]=true;
            }
            enqueue_list(connected_components_subgraph,sg);
            sg=init_sub_graph();
            component_index++;
            clear_list(&edge_stack);
            clear_list(&s);
        }
    }
    if(sg->size==0)
        free(sg);
    free(added); 
    free(low); 
    free(discovery);
    free(visited);
    return connected_components_subgraph;
}

