#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include <json-c/json.h>
#include <stdio.h>
#include <string.h>

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


typedef struct topology * topology_t;

typedef struct node * node_t;

/* topology structures definition*/
struct topology {
	int id_lenght;
	char *protocol;
	char *self_id;
	struct node *first;
};

struct node {
	char *id;
	struct neighbor *neighbor_list;
	node_t next;
	struct local_address *addresses;
};

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

int  add_neigh(topology_t topo, const char *source, const char *id, const double weight, int validity);
int  add_node(topology_t topo, const char *id);

struct neighbor* find_neigh(node_t source, node_t target);
node_t           find_node(topology_t topo, const char *id);

void free_bc_degree_map(map_id_degree_bc * map);

topology_t new_topo(int type);
void       free_topo(topology_t topo);
topology_t parse_jsoninfo(char *buffer);
topology_t parse_netjson(char* buffer);

#endif /* SRC_PARSER_H_ */
