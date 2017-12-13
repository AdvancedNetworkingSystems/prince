#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include <json-c/json.h>
#include <stdio.h>
#include <string.h>

/* bc degree structures definition*/
typedef struct _id_degree_bc {
        char *id;
        double bc;
        int degree;

} id_degree_bc;

typedef struct _map_id_degree_bc {
        id_degree_bc *map;
        size_t size;
        int n_edges;
} map_id_degree_bc;

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
        struct node *next;
        struct local_address *addresses;
};

struct neighbor {
        struct node *id;
        float weight;
        int validity;
        struct neighbor *next;
};

struct local_address {
        const char *id;
        struct local_address *next;
};
struct topology *parse_jsoninfo(char *buffer);
struct topology *parse_netjson(char *buffer);
int add_node(struct topology *topo, const char *id);
struct topology *_init_topo(int type);
int add_neigh(struct topology *topo, const char *source, const char *id,
              const double weight, int validity);
void destroy_topo(struct topology *topo);
struct node *find_node(struct topology *topo, const char *id);
struct neighbor *find_neigh(struct node *source, struct node *target);
void bc_degree_map_delete(map_id_degree_bc *map);

void destroy_topo(struct topology *topo);

#endif /* SRC_PARSER_H_ */
