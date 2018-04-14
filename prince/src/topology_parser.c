#include "topology_parser.h"

#include "topology.h"

/**
* Free a bc_degree map data structure
* @param map_id_degree_bc*  pointer to the data structure
*/
void free_bc_degree_map(map_id_degree_bc *map)
{
	if (map != 0) {
		int i;
		for (i = 0; i < map->size; i++) {
			free(map->map[i].id);
		}
		if (map->map != 0)
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
	topology_t result = new_topo(0);
	if (result == INVALID_TOPOLOGY) {
		fprintf(stderr, "Could not create recieving topology\n");
		return INVALID_TOPOLOGY;
	}
	json_object *topo = json_tokener_parse(buffer);
	if (topo == NULL) {
		fprintf(stderr, "Could not tokenize buffer\n");
		return INVALID_TOPOLOGY;
	}
	json_object_object_foreach(topo, key, val)
	{
		if (strcmp(key, "config") == 0) {
			json_object *config;
			json_object_object_get_ex(topo, key, &config);
			json_object_object_foreach(
				config, key,
				val) if (strcmp(key, "mainIp") == 0)
			{
				result->self_id =
					strdup(json_object_get_string(val));
			}
		} else if (strcmp(key, "topology") == 0) {
			int i;
			json_object *jarray;
			json_object_object_get_ex(topo, key, &jarray);
			int arraylen = json_object_array_length(jarray);
			if (arraylen == 0) {
				return INVALID_TOPOLOGY;
			}
			for (i = 0; i < arraylen; i++) {
				const char *source = 0, *target = 0;
				double cost = 0;
				int validity = 0;
				json_object *elem =
					json_object_array_get_idx(jarray, i);
				json_object_object_foreach(elem, key, val)
				{
					if (strcmp(key, "lastHopIP") == 0) {
						source = json_object_get_string(
							val);
					} else if (strcmp(key, "destinationIP")
						   == 0) {
						target = json_object_get_string(
							val);
					} else if (strcmp(key, "tcEdgeCost")
						   == 0) {
						cost = json_object_get_double(
							val);
					} else if (strcmp(key, "validityTime")
						   == 0) {
						validity = json_object_get_int(
							val);
					} else if (source && target && cost
						   && validity) {
						if (!find_node(result,
							       source)) {
							add_node(result,
								 source);
						}
						if (!find_node(result,
							       target)) {
							add_node(result,
								 target);
						}
						// printf("%s\t%s\t%f\n",
						// source, target, cost);
						if (add_neigh(result, source,
							      target, cost,
							      validity)) {
							printf("error\n");
							return 0;
						}
						source = target = 0;
						cost = 0;
					} else {
						fprintf(stderr,
							"Recieved unknown key '%s'\n",
							key);
					}
				}
			}
		} else {
			fprintf(stderr,
				"Recieved unknown key '%s' when parsing jsoninfo\n",
				key);
		}
	}
	result->protocol = strdup("olsrv1");
	json_object_put(topo);
	return result;
}


struct neighbor *find_neigh(node_t source, node_t target)
{
	struct neighbor *punt;
	for (punt = source->neighbor_list; punt != 0; punt = punt->next) {
		if (punt->id == target) {
			return punt;
		}
	}
	for (punt = target->neighbor_list; punt != 0; punt = punt->next) {
		if (punt->id == source) {
			return punt;
		}
	}
	return NULL;
}


int add_local_address(node_t node, const char *address)
{
	struct local_address *la_temp;
	la_temp = node->addresses;
	node->addresses =
		(struct local_address *)malloc(sizeof(struct local_address));
	node->addresses->id = address;
	node->addresses->next = la_temp;
	return 1;
}


/**
* Parse NetJson format
* @param char* buffer containing the serialized json
* @return representation of the graph as "struct topology*" type
*/
topology_t parse_netjson(char *buffer)
{
	topology_t c_topo = new_topo(0);
	if (c_topo == INVALID_TOPOLOGY) {
		fprintf(stderr, "Could not create recieving topology\n");
		return c_topo;
	}
	json_object *topo = json_tokener_parse(buffer);
	if (topo == NULL) {
		fprintf(stderr, "Could not tokenize buffer\n");
		return INVALID_TOPOLOGY;
	}
	json_object_object_foreach(topo, key, val)
	{
		if (strcmp(key, "protocol") == 0) {
			c_topo->protocol = strdup(json_object_get_string(val));
		} else if (strcmp(key, "router_id") == 0) {
			c_topo->self_id = strdup(json_object_get_string(val));
		} else if (strcmp(key, "nodes") == 0) {
			int i, arraylen;
			json_object *array;
			json_object_object_get_ex(topo, key, &array);
			arraylen = json_object_array_length(array);
			for (i = 0; i < arraylen; i++) {
				const char *node_id;
				json_object *elem =
					json_object_array_get_idx(array, i);
				json_object_object_foreach(elem, key, val)
				{
					if (strcmp(key, "id") == 0) {
						node_id =
							json_object_get_string(
								val);
						add_node(c_topo, node_id);
					} else if (strcmp(key,
							  "local_addresses")
						   == 0) {
						int j, la_len;
						json_object *la_array;
						json_object_object_get_ex(
							elem, key, &la_array);
						la_len =
							json_object_array_length(
								la_array);
						for (j = 0; j < la_len; j++) {
							json_object *la_elem =
								json_object_array_get_idx(
									la_array,
									j);
							node_t node = find_node(
								c_topo,
								node_id);
							if (node
							    == INVALID_NODE) {
								fprintf(stderr,
									"Could not find node %s\n",
									node_id);
							}
							add_local_address(
								node,
								json_object_get_string(
									la_elem));
						}
					}
				}
			}
		} else if (strcmp(key, "links") == 0) {
			int i;
			json_object *jarray;
			json_object_object_get_ex(topo, key, &jarray);
			int arraylen = json_object_array_length(jarray);
			if (arraylen == 0) {
				return 0;
			}
			for (i = 0; i < arraylen; i++) {
				const char *source = NULL, *target = NULL;
				double cost = 0;
				json_object *elem =
					json_object_array_get_idx(jarray, i);
				json_object_object_foreach(elem, key, val)
				{
					if (strcmp(key, "source") == 0) {
						source = json_object_get_string(
							val);
					}
					if (strcmp(key, "target") == 0) {
						target = json_object_get_string(
							val);
					}
					if (strcmp(key, "cost") == 0) {
						cost = json_object_get_double(
							val);
					}
					if (source && target && cost) {
						if (add_neigh(c_topo, source,
							      target, cost,
							      0)) {
							fprintf(stderr,
								"error\n");
							return 0;
						}

						source = target = 0;
						cost = 0;
					}
				}
			}
		} else {
			fprintf(stderr,
				"Recieved an unknown key '%s' when parsing netjson\n",
				key);
		}
	}
	json_object_put(topo);
	return c_topo;
}
