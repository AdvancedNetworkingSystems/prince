/*
 * parser.h
 *
 *  Created on: 02 lug 2016
 *      Author: gabriel
 */

#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include <json-c/json.h>
#include <stdio.h>
#include <string.h>

struct topology{
	int id_lenght;
	char *protocol;
	char *self_id;
	struct node *first;

};

struct node{
	char *id;
	struct neighbor *neighbor_list;
	struct node *next;
};


struct neighbor{
	struct node *id;
	float weight;
	struct neighbor *next;
};


int parse_netjson(char* buffer);
int add_node(struct topology * topo, const char *id);
struct topology * init_topo(int type);
int add_neigh(struct topology *topo, const char *source, const char *id, const double weight);

struct node* find_node(struct topology *topo, const char *id);


#endif /* SRC_PARSER_H_ */
