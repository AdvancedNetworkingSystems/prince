/* Graph_Parser_lib.h */

#include "topology_parser.h"
#include "brandes.h"

/**
 * Whether we will use recursion or iteration.
 * Used expecially for devices with small computation power or low core number.
 */
extern bool recursive;

/**
 * Whether we want to run on multiple threads.
 * It can be set manually or either you can set it via code, detecting number
 * of cores.
 * Default is false.
 */
extern bool multithread;

/**
 * Whether we compute centrality only if the network changed or all the time.
 * We define a base measure of change, and will be further studied.
 * Default is true.
 */
extern bool  stop_computing_if_unchanged;
typedef void c_graph_parser;

struct graph_parser
{
    struct graph g;


    bool     heuristic_b;
    double * bc;
};


c_graph_parser * new_graph_parser(int weight,
                                  int heuristic);

void graph_parser_parse_simplegraph(c_graph_parser * v, topology_t topo);

void graph_parser_calculate_bc(c_graph_parser * v);

int graph_parser_compose_degree_bc_map(c_graph_parser * v,
        map_id_degree_bc *                              map);

void free_graph_parser(c_graph_parser * v);
