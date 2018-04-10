#include "parser.h"

/**
* Delete a bc_degree map data structure
* @param map_id_degree_bc*  pointer to the data structure
*/
void bc_degree_map_delete(map_id_degree_bc * map)
{
	if(map!=0){
		int i;
		for(i=0; i<map->size; i++){
			free(map->map[i].id);
		}
		if(map->map!=0)
		free(map->map);
		free(map);
	}
}

/**
* Parse jsoninfo format
* @param char* buffer containing the serialized json
*/
topology_t parse_jsoninfo(char *buffer)
{
	topology_t c_topo= new_topo(0);
	json_object *topo = json_tokener_parse(buffer);
	if(!topo) return 0;
	json_object_object_foreach(topo, key, val) {
		if(strcmp(key,"config")==0){
			json_object *config;
			json_object_object_get_ex(topo, key, &config);
			json_object_object_foreach(config, key, val)
				if(strcmp(key, "mainIp")==0)
					c_topo->self_id=strdup(json_object_get_string(val));
		}
		if(strcmp(key, "topology")==0){
			int i;
			json_object *jarray;
			json_object_object_get_ex(topo, key, &jarray);
			int arraylen = json_object_array_length(jarray);
			if(arraylen ==0)
				return 0;
			for(i=0; i<arraylen; i++){
				const char *source=0, *target=0;
				double cost=0;
				int validity=0;
				json_object *elem =json_object_array_get_idx(jarray,i);
				json_object_object_foreach(elem, key, val) {
					if(strcmp(key, "lastHopIP")==0){
						source=json_object_get_string(val);
					}
					if(strcmp(key, "destinationIP")==0){
						target=json_object_get_string(val);
					}
					if(strcmp(key, "tcEdgeCost")==0){
						cost=json_object_get_double(val);
					}
					if(strcmp(key, "validityTime")==0){
						validity=json_object_get_int(val);
					}
					if(source && target && cost && validity){
						if(!find_node(c_topo, source))
							add_node(c_topo, source);
						if(!find_node(c_topo, target))
							add_node(c_topo, target);
						//printf("%s\t%s\t%f\n", source, target, cost);
						if(!add_neigh(c_topo, source, target, cost, validity)){
							printf("error\n");
							return 0;
						}
						source = target =0;
						cost =0;
					}
				}
			}
		}
	}
	json_object_put(topo);
	return c_topo;
}

/**
* Add a node to the topology data structure
* @param struct topology*  pointer to the topology data structure
* @param const char* string containing the id of the new node
* @return 1 on success, 0 otherwise
*/
int add_node(topology_t topo, const char *id)
{
	node_t temp = topo->first;
	topo->first=(node_t) malloc(sizeof(struct node));
	topo->first->next=temp;
	topo->first->id=strdup(id);
	topo->first->neighbor_list=0;
	topo->first->addresses=0;
	return 1;
}

struct neighbor* find_neigh(node_t source, node_t target){
	struct neighbor *punt;
	for(punt=source->neighbor_list; punt!=0; punt=punt->next){
		if(punt->id==target)
			return punt;
	}
	for(punt=target->neighbor_list; punt!=0; punt=punt->next){
		if(punt->id==source)
			return punt;
	}
	return 0;
}

/**
* Find a node in the topology data structure
* @param struct topology*  pointer to the topology data structure
* @param const char* string containing the id of the searched node
* @return pointer to the node on success, 0 otherwise
*/
node_t find_node(topology_t topo,const char *id)
{
	node_t punt;
	for(punt=topo->first; punt!=0; punt=punt->next){
		if(strcmp(punt->id, id)==0){
			return punt;
		}
		struct local_address *la;
		for(la=punt->addresses; la!=0; la=la->next){
			if(strcmp(la->id, id)==0){
				return punt;
			}
		}
	}
	return 0;
}

/**
* Add a neighbor to the node
* @param struct topology*  pointer to the topology data structure
* @param const char* string containing the id of the source node
* @param const char* string containing the id of the target node
* @param const double  cost of the edge
* @return 1 on success, 0 otherwise
*/
int add_neigh(topology_t topo, const char *source, const char *id, const double weight, int validity)
{
	struct neighbor *temp, *found;
	node_t s, t;
	if((s=find_node(topo, source))==0)
		return 0; // check if source node exists
	if((t=find_node(topo, id))==0)
		return 0; //check if target node exists
	found=find_neigh(s, t);
	if(found){
		if(found->validity > validity)
			found->weight = weight; //if the link found is older, i update the weight
		return 1; //The link is already present
	}

	temp=s->neighbor_list;
	s->neighbor_list=(struct neighbor*)malloc(sizeof(struct neighbor));
	s->neighbor_list->id=t; // add node to source neighbor list
	s->neighbor_list->weight=weight;
	s->neighbor_list->validity = validity;
	s->neighbor_list->next=temp;
	return 1;
}

int add_local_address(node_t node, const char* address){
	struct local_address *la_temp;
	la_temp = node->addresses;
	node->addresses =(struct local_address*)malloc(sizeof(struct local_address));
	node->addresses->id=address;
	node->addresses->next=la_temp;
	return 1;
}
/**
* Initialize the topology data structure
* @param int number of chars of the id (0 ipv6, 1 ipv4)
* @return pointer to the topology
*/
topology_t new_topo(int type)
{
	topology_t topo = (topology_t) malloc(sizeof(struct topology));
	if(type==0){
		topo->id_lenght=39;
	}else if(type ==1){
		topo->id_lenght=15;
	}
	topo->protocol=0;
	topo->first=0;
	return topo;
}


/**
* Destroy topology and dealloc
* @param struct topology * pointer to the structure
**/
void free_topo(topology_t topo)
{
	node_t n_temp, punt=topo->first;
	while (punt) {
		struct neighbor *n=punt->neighbor_list;
		while(n){
			struct neighbor *temp=n->next;
			free(n);
			n=temp;
		}
		free(punt->id);
		n_temp=punt->next;
		free(punt);
		punt=n_temp;
	}
	free(topo->protocol);
	free(topo->self_id);
	free(topo);
}

/**
* Parse NetJson format
* @param char* buffer containing the serialized json
* @return representation of the graph as "struct topology*" type
*/
topology_t parse_netjson(char* buffer)
{
	topology_t c_topo= new_topo(0);
	json_object *topo = json_tokener_parse(buffer);
	if(!topo) return 0;
	json_object_object_foreach(topo, key, val) {
		if(strcmp(key, "protocol")==0)
		c_topo->protocol=strdup(json_object_get_string(val));
		if(strcmp(key,"router_id")==0){
			c_topo->self_id=strdup(json_object_get_string(val));
		}
		if(strcmp(key, "nodes")==0){
			int i, arraylen;
			json_object *array;
			json_object_object_get_ex(topo, key, &array);
			arraylen = json_object_array_length(array);
			for(i=0; i<arraylen; i++){
				const char *node_id;
				json_object *elem =json_object_array_get_idx(array,i);
				json_object_object_foreach(elem, key, val){
					if(strcmp(key, "id")==0){
						node_id = json_object_get_string(val);
						add_node(c_topo, node_id);
					}else if(strcmp(key, "local_addresses")==0){
						int j, la_len;
						json_object *la_array;
						json_object_object_get_ex(elem, key, &la_array);
						la_len = json_object_array_length(la_array);
							for(j=0; j<la_len; j++){
								json_object *la_elem =json_object_array_get_idx(la_array, j);
								node_t node = find_node(c_topo, node_id);
								add_local_address(node, json_object_get_string(la_elem));
							}
					}

				}
			}
		}
		if(strcmp(key, "links")==0){
			int i;
			json_object *jarray;
			json_object_object_get_ex(topo, key, &jarray);
			int arraylen = json_object_array_length(jarray);
			if(arraylen ==0)
				return 0;
			for(i=0; i<arraylen; i++){
				const char *source=0, *target=0;
				double cost=0;
				json_object *elem =json_object_array_get_idx(jarray,i);
				json_object_object_foreach(elem, key, val) {
					if(strcmp(key, "source")==0){
						source=json_object_get_string(val);
					}
					if(strcmp(key, "target")==0){
						target=json_object_get_string(val);
					}
					if(strcmp(key, "cost")==0){
						cost=json_object_get_double(val);
					}
					if(source && target && cost){
						/*printf("   %s %s %f\n", source, target, cost);*/
						if(!add_neigh(c_topo, source, target, cost, 0)){
							printf("error\n");
							return 0;
						}

						source = target =0;
						cost =0;
					}
				}

			}
		}
	}
	json_object_put(topo);
	return c_topo;
}
