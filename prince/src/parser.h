/*
Copyright (c) 2016 Gabriele Gemmi <gabriel@autistici.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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

/* bc degree structures definition*/
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



/* topology structures definition*/
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

struct topology * parse_jsoninfo(char *buffer);
struct topology * parse_netjson(char* buffer);
int add_node(struct topology * topo, const char *id);
struct topology * _init_topo(int type);
int add_neigh(struct topology *topo, const char *source, const char *id, const double weight);

struct node* find_node(struct topology *topo, const char *id);
void bc_degree_map_delete(map_id_degree_bc * map);

void destroy_topo(struct topology *topo);

#endif /* SRC_PARSER_H_ */
