/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "graph_parser.h"
#include "algorithms.h"



c_graph_parser* new_graph_parser(int weight, int heuristic){
    struct graph * g=(struct graph *)malloc(sizeof(struct graph));
    init_graph(g);
    return g;
}
void graph_parser_parse_simplegraph(c_graph_parser* v, struct topology *topo){
    struct graph * g=(struct graph *)v;
    struct node *punt;
    for(punt=topo->first; punt!=0; punt=punt->next){
        struct neighbor* neigh;
        for(neigh=punt->neighbor_list; neigh!=0; neigh=neigh->next){
            const char * source = punt->id;
            const char * target = neigh->id->id;
            double cost = neigh->weight;
            add_edge_graph(g,source,target, cost);
        }
    }
}
void graph_parser_calculate_bc(void* v){
    struct graph * g=(struct graph *)v;
    betweeness_brandes(g);
    
}
int graph_parser_compose_degree_bc_map(c_graph_parser* v, map_id_degree_bc *map){
    printf("graph_parser_compose_degree_bc_map empty\n");
}
void delete_graph_parser(void* v){
    free_graph(v);
}