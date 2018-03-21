#ifndef TOPOLOGY_H_
#define TOPOLOGY_H_

#define INVALID_TOPOLOGY NULL

typedef struct topology * topology_t;
typedef struct node * node_t;

struct node {
	char *id;
	struct neighbor *neighbor_list;
	node_t next;
	struct local_address *addresses;
};

struct topology {
	int id_lenght;
	char *protocol;
	char *self_id;
	struct node *first;
};

topology_t new_topo(int type);
void       free_topo(const topology_t topo);
node_t     find_node(const topology_t topo, const char *id);
int        add_node(topology_t topo, const char *id);
int        add_neigh(topology_t topo, const char *source, const char *id, const double weight, int validity);
#endif /*TOPOLOGY_H_ */
