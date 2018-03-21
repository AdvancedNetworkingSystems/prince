#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include <json-c/json.h>
#include <stdio.h>
#include <string.h>

#include "topology.h"

/* bc degree structures definition*/
typedef struct _id_degree_bc{
	char* id;
	double bc;
	int degree;

} id_degree_bc;

typedef struct _map_id_degree_bc{
	id_degree_bc *map;
	size_t size;
	int n_edges;
} map_id_degree_bc;


struct neighbor {
	node_t id;
	float weight;
	int validity;
	struct neighbor *next;
};

struct local_address {
	const char *id;
	struct local_address *next;
};

struct neighbor* find_neigh(node_t source, node_t target);

void free_bc_degree_map(map_id_degree_bc * map);

topology_t parse_jsoninfo(char *buffer);
topology_t parse_netjson(char* buffer);

#endif /* SRC_PARSER_H_ */
