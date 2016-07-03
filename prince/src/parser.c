#include "parser.h"

int
parse_jsoninfo(char *buffer){
	struct topology *c_topo= init_topo(1);
	return 1;
}

int
add_node(struct topology * topo, const char *id){
	struct node *temp = topo->first;
	topo->first=malloc(sizeof(struct node));
	topo->first->next=temp;
	topo->first->id=strdup(id);
	topo->first->neighbor_list=0;
	return 1;
}


struct node*
find_node(struct topology *topo,const char *id){
	struct node *punt;
	for(punt=topo->first; punt->next!=0; punt=punt->next){
		if(strcmp(punt->id, id)==0){
			return punt;
		}
	}
	return 0;
}

int
add_neigh(struct topology *topo, const char *source, const char *id, const double weight){
	struct neighbor *temp;
	struct node* n;
	if((n=find_node(topo, source))==0)
		return 0;

	temp=n->neighbor_list;
	n->neighbor_list=malloc(sizeof(struct neighbor));
	if((n->neighbor_list->id=find_node(topo, id))==0)
		return 0;
	n->neighbor_list->weight=weight;
	n->neighbor_list->next=temp;
	return 1;

}


struct topology * init_topo(int type){
	struct topology *topo = malloc(sizeof(struct topology));
	if(type==0){
		topo->id_lenght=39;
	}else if(type ==1){
		topo->id_lenght=15;
	}
	topo->first=0;
	return topo;
}

struct topology *
parse_netjson(char* buffer){
	enum json_type type;

	struct topology *c_topo= init_topo(0);
	json_object *topo = json_tokener_parse(buffer);
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
				json_object *elem =json_object_array_get_idx(array,i);
				json_object_object_foreach(elem, key, val){
					if(strcmp(key, "id")==0){
						add_node(c_topo, strdup(json_object_get_string(val)));
					}

				}
			}


		}

		if(strcmp(key, "links")==0){
			int i;
			json_object *jarray;
			json_object_object_get_ex(topo, key, &jarray);
			int arraylen = json_object_array_length(jarray);
			for(i=0; i<arraylen; i++){
				json_object *elem =json_object_array_get_idx(jarray,i);
				json_object_object_foreach(elem, key, val) {
					const char *source, *target;
					double cost;
					if(strcmp(key, "source")==0){
						source=json_object_get_string(val);
					}
					if(strcmp(key, "target")==0){
						target=json_object_get_string(val);
					}
					if(strcmp(key, "cost")==0){
						cost=json_object_get_double(val);
					}
					add_neigh(c_topo, source, target, cost);
				}

			}
		}
	}
	return c_topo;
}
