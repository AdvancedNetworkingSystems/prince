#include "graph_parser.h"

bool recursive=true;



c_graph_parser* new_graph_parser(int weight, int heuristic){
    struct graph_parser * gp=(struct graph_parser *)malloc(sizeof(struct graph_parser));
    gp->heuristic_b=heuristic==1;
    gp->bc=0;
    init_graph(&(gp->g));
    return (c_graph_parser*)gp;
}
void graph_parser_parse_simplegraph(c_graph_parser* v, struct topology *topo){
    struct graph_parser * gp=(struct graph_parser *)v;
    struct node *punt;
    free_graph(&(gp->g));
    init_graph(&(gp->g));
    for(punt=topo->first; punt!=0; punt=punt->next){
        struct neighbor* neigh;
        
        for(neigh=punt->neighbor_list; neigh!=0; neigh=neigh->next){
            const char* source = punt->id;
            const char* target = neigh->id->id;
            double cost = neigh->weight;
            add_edge_graph(&(gp->g),source,target, cost,false);
            
        }
    }
}


void graph_parser_calculate_bc(c_graph_parser* v){
    struct graph_parser * gp=(struct graph_parser *)v;
    if(gp->heuristic_b&&gp->g.nodes.size>80 || true){//TODO check here
        gp->bc=(double*)betwenness_heuristic(&(gp->g),recursive);
    }else{
        gp->bc=betweeness_brandes(&(gp->g),true,0);
    }
}
int graph_parser_compose_degree_bc_map(c_graph_parser* v, map_id_degree_bc *map){
    struct graph_parser * gp=(struct graph_parser *)v;
    map->size=gp->g.nodes.size;
    map->map = (struct _id_degree_bc *) malloc(sizeof(struct _id_degree_bc)*gp->g.nodes.size);
    map->n_edges = 0; //Todo: add counting
    struct node_list *  nl;
    int i=0;
    for(nl=gp->g.nodes.head;nl!=0;nl=nl->next){
        struct node_graph * ng=(struct node_graph*)nl->content;
        map->map[i].id = strdup(ng->name) ;
        map->map[i].bc = gp->bc[ng->node_graph_id];
        map->n_edges+=ng->neighbours.size;
        map->map[i].degree = ng->neighbours.size;
        i++;
    }
    return 1;
}

void delete_graph_parser(void* v){
    struct graph_parser * gp=(struct graph_parser *)v;
    free_graph(&gp->g);
    if(gp->bc!=0)
    free(gp->bc);
    free(gp);
    //gp->bc=0;
}
