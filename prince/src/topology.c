#include "topology.h"

#include <stdio.h>
#include <stdlib.h>

#include "graph_parser.h"

/**
* Add a node to the topology data structure
* @param struct topology*  pointer to the topology data structure
* @param const char* string containing the id of the new node
* @return 1 on success, 0 otherwise
*/
int add_node(topology_t topo, const char *id)
{
	node_t head = topo->first;
	topo->first = (node_t)malloc(NODE_SIZE);
	if (topo->first == INVALID_NODE) {
		fprintf(stderr,
			"prince-topology: could not add node '%s' to topology '%s'",
			id, topo->self_id);
		return 0;
	}
	topo->first->addresses = 0;
	topo->first->id = strdup(id);
	topo->first->neighbor_list = 0;
	topo->first->next = head;
	return 1;
}

/**
* Find a node in the topology data structure
* @param struct topology*  pointer to the topology data structure
* @param const char* string containing the id of the searched node
* @return pointer to the node on success, 0 otherwise
*/
node_t find_node(topology_t topo, const char *id)
{
	node_t punt;
	for (punt = topo->first; punt != INVALID_NODE; punt = punt->next) {
		if (strcmp(punt->id, id) == 0) {
			return punt;
		}
		struct local_address *address;
		for (address = punt->addresses; address != NULL;
		     address = address->next) {
			if (strcmp(address->id, id) == 0) {
				return punt;
			}
		}
	}
	return INVALID_NODE;
}

/**
* Add a neighbor to the node
* @param struct topology*  pointer to the topology data structure
* @param const char* string containing the id of the source node
* @param const char* string containing the id of the target node
* @param const double  cost of the edge
* @return 0 on success, 1 otherwise
*/
int add_neigh(topology_t topo, const char *source, const char *id,
	      const double weight, int validity)
{
	struct neighbor *temp, *found;
	node_t s, t;
	if ((s = find_node(topo, source)) == NULL)
		return 1; // check if source node exists
	if ((t = find_node(topo, id)) == NULL)
		return 1; // check if target node exists
	found = find_neigh(s, t);
	if (found) {
		if (found->validity > validity) {
			found->weight = weight; // if the link found is older, i
						// update the weight
		}
		return 0; // The link is already present
	}

	temp = s->neighbor_list;
	s->neighbor_list = (struct neighbor *)malloc(sizeof(struct neighbor));
	if (s->neighbor_list == NULL) {
		perror("topology");
		return 1;
	}
	s->neighbor_list->id = t; // add node to source neighbor list
	s->neighbor_list->next = temp;
	s->neighbor_list->validity = validity;
	s->neighbor_list->weight = weight;
	return 0;
}

/**
* Initialize the topology data structure
* @param int number of chars of the id (0 ipv6, 1 ipv4)
* @return pointer to the topology
*/
topology_t new_topo(int topology_type)
{
	topology_t topo = (topology_t)malloc(TOPOLOGY_SIZE);
	if (topo == INVALID_TOPOLOGY) {
		perror("topology");
		return NULL;
	}
	switch (topology_type) {
	case 0:
		topo->id_lenght = 39;
		break;
	case 1:
		topo->id_lenght = 15;
		break;
	default:
		fprintf(stderr, "Received an unknown topology type <%d>\n",
			topology_type);
		free_topo(topo);
		return NULL;
	}

	topo->first = 0;
	topo->protocol = 0;
	return topo;
}


/**
* Free topology and dealloc
* @param struct topology * pointer to the structure
**/
void free_topo(topology_t topo)
{
	node_t n_temp, punt = topo->first;
	while (punt) {
		struct neighbor *n = punt->neighbor_list;
		while (n) {
			struct neighbor *temp = n->next;
			free(n);
			n = temp;
		}
		free(punt->id);
		n_temp = punt->next;
		free(punt);
		punt = n_temp;
	}
	free(topo->protocol);
	free(topo->self_id);
	free(topo);
}

int valid_topo(const topology_t topo)
{
	if (topo->self_id == NULL) {
		fprintf(stderr, "Topology does not have valid id\n");
		return 1;
	}
	if (topo->protocol == NULL) {
		fprintf(stderr, "Topology does not have valid protocol\n");
		return 1;
	}
	if (topo->first == NULL) {
		fprintf(stderr, "Topology does not have valid node list\n");
		return 1;
	}
	return 0;
}
