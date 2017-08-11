#include "config.h"

char* read_file_content(char *filename)
{
	char *buffer = NULL;
	int string_size, read_size;
	FILE *handler = fopen(filename, "r");

	if (handler)
	{

		fseek(handler, 0, SEEK_END);
		string_size = ftell(handler);
		rewind(handler);

		buffer = (char*) malloc(sizeof(char) * (string_size + 1) );

		read_size = fread(buffer, sizeof(char), string_size, handler);

		buffer[string_size] = '\0';

		if (string_size != read_size)
		{
			free(buffer);
			buffer = NULL;
		}

		fclose(handler);
	}

	return buffer;
}


/**
* Given a json in the following format:
* {
*    "proto":{
*       "protocol":"oonf",
*       "host":"127.0.0.1",
*       "port":2019,
*       "refresh":1
*    },
*    "graph-parser":{
*       "heuristic":1,
*       "weights":0,
*       "recursive":1,
*       "stop_unchanged":0,
*       "multithreaded":1
*    }
* }
* it parses it and initializes the program parameter
* @param filepath A json file path
* @param ph The prince_handler, for saving the configuration
* @return Whether the reading ended successfully
*/
bool parse_json_config(char *filepath,struct prince_handler *ph)
{
	char * buffer = read_file_content(filepath);
	int completed=0;
	bool heuristic_set=false,weights_set=false,
	recursive_set=false,stop_unchanged_set=false,multithreaded_set=false;
	if(!buffer) return false;
	struct json_object *jobj = json_tokener_parse(buffer);
	json_object_object_foreach(jobj, key, val) {
		if(strcmp(key, "proto")==0){
			if(json_object_get_type(val)==json_type_object){
				json_object_object_foreach(val, key_i, val_i) {
					if(ph->proto<0&&strcmp(key_i, "protocol")==0){
						if(json_object_get_type(val_i)==json_type_string){
							const char * content=json_object_get_string(val_i);
							ph->proto = strdup(content);
							completed++;
						}
					}else if(ph->host==0&&strcmp(key_i, "host")==0){
						if(json_object_get_type(val_i)==json_type_string){
							const char * content=json_object_get_string(val_i);
							ph->host=strdup(content);
							completed++;

						}
					}else if(ph->port<0&&strcmp(key_i, "port")==0){
						if(json_object_get_type(val_i)==json_type_int){
							ph->port=json_object_get_int(val_i);
							completed++;
						}
					}
					else if(strcmp(key_i, "timer_port")==0){
						if(json_object_get_type(val_i)==json_type_int){
							ph->timer_port=json_object_get_int(val_i);
							completed++;
						}
					}else if(strcmp(key_i, "refresh")==0){
						if(ph->refresh<0&&json_object_get_type(val_i)==json_type_int){
							ph->refresh=json_object_get_int(val_i);
							completed++;
						}
					}
				}
			}
		}else if(strcmp(key, "graph-parser")==0){
			if(json_object_get_type(val)==json_type_object){
				json_object_object_foreach(val, key_i, val_i) {
					if(!heuristic_set&&strcmp(key_i, "heuristic")==0){
						if(json_object_get_type(val_i)==json_type_int){
							ph->heuristic=json_object_get_int(val_i)==1;
							heuristic_set=true;
						}
					}else if(strcmp(key_i, "weights")==0){
						if(!weights_set&&json_object_get_type(val_i)==json_type_int){
							ph->weights=json_object_get_int(val_i)==1;
							weights_set=true;
						}
					}else if(strcmp(key_i, "recursive")==0){
						if(!recursive_set&&json_object_get_type(val_i)==json_type_int){
							ph->recursive=json_object_get_int(val_i)==1;
							recursive_set=true;
						}
					}else if(strcmp(key_i, "stop_unchanged")==0){
						if(!stop_unchanged_set&&json_object_get_type(val_i)==json_type_int){
							ph->stop_unchanged=json_object_get_int(val_i);
							stop_unchanged_set=true;
						}
					}else if(strcmp(key_i, "multithreaded")==0){
						if(!multithreaded_set&&json_object_get_type(val_i)==json_type_int){
							ph->multithreaded=json_object_get_int(val_i);
							multithreaded_set=true;
						}
					}
				}
			}
		}
	}
	if(jobj!=0){
		json_object_put(jobj);
		/* json_object_object_del(jobj, "");*/
	}
	free(buffer);
	if(completed==5&&heuristic_set && weights_set && recursive_set
		&& stop_unchanged_set && multithreaded_set)
		return true;
	return false;
}

	/**
	* Read the ini configuration and populate struct prince_handler
	* @param *ph pinter to the prince_handler object
	* @param *filepath path to the configuration file
	* @return 1 if success, 0 if fail
	*/
	int
	read_config_file(struct prince_handler *ph, char *filepath){
		if(!parse_json_config(filepath, ph)) {
			printf("Cannot read configuration file '%s' (Either format or content not compliant or complete). Exiting.\n", filepath);
			return 0;
		}

		printf("Config loaded from %s\n", filepath);
		return 1;
	}
