/* Graph_Parser_lib.h*/

#include "../../prince/src/parser.h"


typedef void c_graph_parser;
/*
typedef struct _id_degree_bc{
    char* id;
    double bc;
    int degree;
    
}id_degree_bc;

typedef struct _map_id_degree_bc{
    id_degree_bc *map;
    size_t size;
    int n_edges;
}map_id_degree_bc;


struct topology{
    int id_lenght;
    const char *protocol;
    const char *self_id;
    struct node *first;
    
};

struct node{
    const char *id;
    struct neighbor *neighbor_list;
    struct node *next;
};


struct neighbor{
    struct node *id;
    float weight;
    struct neighbor *next;
};
*/
c_graph_parser* new_graph_parser(int weight, int heuristic);
void graph_parser_parse_simplegraph(c_graph_parser* v,struct topology *topo);
void graph_parser_calculate_bc(c_graph_parser* v);
int graph_parser_compose_degree_bc_map(c_graph_parser* v, map_id_degree_bc *map);
void delete_graph_parser(c_graph_parser* v);
