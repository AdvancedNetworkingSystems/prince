
#include "graph_parser.h"

bool recursive = true;

/**
 * Wrapper function for initializing the computation
 *
 * @param weight This boolean parameter is used only for backward compatibility.
 * In the current version, graphs are all considered weighted
 * @param heuristic This boolean parameter tells whether to use heuristic.
 * @return a struct of type c_graph_parser that is capable of computing
 * Brandes Betweenness centrality
 */
c_graph_parser *new_graph_parser(int weight, int heuristic)
{
	struct graph_parser *gp =
		(struct graph_parser *)malloc(sizeof(struct graph_parser));
	if (gp == NULL) {
		perror("graph-parser-new");
		return NULL;
	}

	gp->heuristic_b = heuristic == 1;
	gp->bc = 0;

	init_graph(&(gp->g));

	return (c_graph_parser *)gp;
}

/**
 * Wrapper function for loading the graph
 *
 * @param v A c_graph_parser struct that contains data to run the centrality
 * algorithm
 * @param topo A topology struct that defines the structure of a graph,
 * in particular the neighbors list
 */
void graph_parser_parse_simplegraph(c_graph_parser *v, struct topology *topo)
{
	struct graph_parser *gp = (struct graph_parser *)v;
	struct node *punt;
	free_graph(&(gp->g));
	init_graph(&(gp->g));

	for (punt = topo->first; punt != 0; punt = punt->next) {
		struct neighbor *neigh;
		for (neigh = punt->neighbor_list; neigh != 0;
		     neigh = neigh->next) {
			const char *source = punt->id;
			const char *target = neigh->id->id;
			double cost = neigh->weight;
			add_edge_graph(&(gp->g), source, target, cost, false);
		}
	}
}

/**
 * Wrapper function for actually computing the centrality
 * @param v c_graph_parser struct that contains data to run the centrality
 * algorithm
 */
void graph_parser_calculate_bc(c_graph_parser *v)
{
	struct graph_parser *gp = (struct graph_parser *)v;
	// empirical results show that in smaller graphs the
	// original algorithm is faster
	if (gp->heuristic_b && (gp->g.nodes.size > 80)) {
		gp->bc = (double *)betwenness_heuristic(&(gp->g), recursive);
	} else {
		gp->bc = betweeness_brandes(&(gp->g), true, 0, false);
	}
}

/**
 * Wrapper function that returns in a custom struct the centrality and other
 * data.
 * @param v c_graph_parser struct that contains data with results
 * @param map A custom struct that contains data relative to the centrality and
 * @return A fixed value
 */
int graph_parser_compose_degree_bc_map(c_graph_parser *v, map_id_degree_bc *map)
{
	struct graph_parser *gp = (struct graph_parser *)v;

	map->size = gp->g.nodes.size;
	map->map = (struct _id_degree_bc *)malloc(sizeof(struct _id_degree_bc)
						  * gp->g.nodes.size);
	map->n_edges = 0;
	struct node_list *nl;


	int i = 0;

	for (nl = gp->g.nodes.head; nl != 0; nl = nl->next) {
		struct node_graph *ng = (struct node_graph *)nl->content;

		map->map[i].id = strdup(ng->name);
		map->map[i].bc = gp->bc[ng->node_graph_id];
		map->n_edges += ng->neighbours.size;
		map->map[i].degree = ng->neighbours.size;
		i++;
	}
	return 1;
}

/**
 * Function that deallocates and frees memory used for centrality computation
 * @param v c_graph_parser struct that has to be deleted
 */
void free_graph_parser(void *v)
{
	struct graph_parser *gp = (struct graph_parser *)v;
	if (gp->bc != NULL) {
		free(gp->bc);
	}
	free(gp);
}
