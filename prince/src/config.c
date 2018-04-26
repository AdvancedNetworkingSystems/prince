#include "config.h"

char *read_file_content(const char *filename)
{
	char *buffer = NULL;
	int string_size, read_size;
	FILE *handler = fopen(filename, "r");

	if (handler == NULL) {
		perror("prince-config");
		exit(EXIT_FAILURE);
	} else {
		fprintf(stdout, "Reading %s\n", filename);
		fseek(handler, 0, SEEK_END);
		string_size = ftell(handler);
		rewind(handler);

		buffer = (char *)malloc(sizeof(char) * (string_size + 1));
		if (buffer == NULL) {
			perror("prince-config");
			return NULL;
		}

		read_size = fread(buffer, sizeof(char), string_size, handler);

		buffer[string_size] = '\0';

		if (string_size != read_size) {
			free(buffer);
			buffer = NULL;
		}
		if (fclose(handler)) {
			perror("prince-config");
			exit(EXIT_FAILURE);
		}
	}
	return buffer;
}

/**
* Read the ini configuration and populate struct prince_handler
* @param *ph pinter to the prince_handler object
* @param *filepath path to the configuration file
* @return 0 if success, -1 if fail
*/
int read_config_file(prince_handler_t ph, const char *filepath)
{
	if (parse_json_config(filepath, ph)) {
		fprintf(stderr,
			"Cannot read configuration file '%s' (Either format or content not compliant or complete). Exiting.\n",
			filepath);
		return -1;
	}

	fprintf(stdout, "Config loaded from %s\n", filepath);
	return 0;
}


/**
* Given a json in the following format:
* {
*    "proto":{
*       "protocol":"oonf",
*       "host":"127.0.0.1",
*       "port":2009,
*       "timer_port": 1234,
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
* @return 0 on success, -1 on failure
*/
int parse_json_config(const char *filepath, prince_handler_t ph)
{
	char *buffer = read_file_content(filepath);
	if (buffer == NULL) {
		return -1;
	}
	int completed = 0;
	bool heuristic_set = false, weights_set = false, recursive_set = false,
	     stop_unchanged_set = false, multithreaded_set = false;
	struct json_object *jobj = json_tokener_parse(buffer);

	if (jobj == NULL) {
		fprintf(stderr, "prince-config: Received null json pointer");
		exit(EXIT_FAILURE);
	}

	json_object_object_foreach(jobj, key, val)
	{
		if (strcmp(key, "proto") == 0) {
			if (json_object_get_type(val) == json_type_object) {
				json_object_object_foreach(val, key_i, val_i)
				{
					if (strcmp(key_i, "protocol") == 0) {
						if (json_object_get_type(val_i)
						    == json_type_string) {
							const char *content =
								json_object_get_string(
									val_i);
							ph->proto =
								strdup(content);
							completed++;
						} else {
							fprintf(stderr,
								"\"protocol\" value is not a string\n");
							exit(EXIT_FAILURE);
						}

					} else if (ph->host == 0
						   && strcmp(key_i, "host")
							      == 0) {
						if (json_object_get_type(val_i)
						    == json_type_string) {
							const char *content =
								json_object_get_string(
									val_i);
							ph->host =
								strdup(content);
							completed++;
						} else {
							fprintf(stderr,
								"\"host\" value is not a string\n");
							exit(EXIT_FAILURE);
						}
					} else if (ph->port < 0
						   && strcmp(key_i, "port")
							      == 0) {
						if (json_object_get_type(val_i)
						    == json_type_int) {
							ph->port =
								json_object_get_int(
									val_i);
							completed++;
						} else {
							fprintf(stderr,
								"\"port\" value is not an int\n");
							exit(EXIT_FAILURE);
						}
					} else if (ph->port < 0
						   && strcmp(key_i, "json_type")
							      == 0) {
						if (json_object_get_type(val_i)
						    == json_type_string) {
							const char *content =
								json_object_get_string(
									val_i);
							if (strcmp(content,
								   "netjson")
							    == 0) {
								ph->json_type =
									1;
							} else if (
								strcmp(content,
								       "jsoninfo")
								== 0) {
								ph->json_type =
									0;
							} else {
								fprintf(stderr,
									"\"json_type\" value is not specified, will default to netjson");
								ph->json_type =
									0;
							}
						} else if (json_object_get_type(
								   val_i)
							   == json_type_int) {
							ph->json_type =
								json_object_get_int(
									val_i);
							completed++;
						} else {
							fprintf(stderr,
								"\"json_type\" value is not accepted\n");
							exit(EXIT_FAILURE);
						}
					} else if (strcmp(key_i, "timer_port")
						   == 0) {
						if (json_object_get_type(val_i)
						    == json_type_int) {
							ph->timer_port =
								json_object_get_int(
									val_i);
							completed++;
						} else {
							fprintf(stderr,
								"\"timer_port\" value is not an int\n");
							exit(EXIT_FAILURE);
						}
					} else if (strcmp(key_i, "refresh")
						   == 0) {
						if (ph->refresh < 0
						    && json_object_get_type(
							       val_i)
							       == json_type_int) {
							ph->refresh =
								json_object_get_int(
									val_i);
							completed++;
						} else {
							fprintf(stderr,
								"\"refresh\" value is not an int\n");
							exit(EXIT_FAILURE);
						}
					} else if (strcmp(key_i, "log_file")
						   == 0) {
						if (json_object_get_type(val_i)
						    == json_type_string) {
							const char *content =
								json_object_get_string(
									val_i);
							ph->log_file =
								strdup(content);
						} else {
							fprintf(stderr,
								"\"log_file\" value is not a string\n");
							exit(EXIT_FAILURE);
						}
					}
				}
			}
		} else if (strcmp(key, "graph-parser") == 0) {
			if (json_object_get_type(val) == json_type_object) {
				json_object_object_foreach(val, key_i, val_i)
				{
					if (!heuristic_set
					    && strcmp(key_i, "heuristic")
						       == 0) {
						if (json_object_get_type(val_i)
						    == json_type_int) {
							ph->heuristic =
								json_object_get_int(
									val_i)
								== 1;
							heuristic_set = true;
						} else {
							fprintf(stderr,
								"\"heuristic\" value not an int\n");
							exit(EXIT_FAILURE);
						}
					} else if (strcmp(key_i, "weights")
						   == 0) {
						if (!weights_set
						    && json_object_get_type(
							       val_i)
							       == json_type_int) {
							ph->weights =
								json_object_get_int(
									val_i)
								== 1;
							weights_set = true;
						} else {
							fprintf(stderr,
								"\"weights\" value not an int\n");
							exit(EXIT_FAILURE);
						}
					} else if (strcmp(key_i, "recursive")
						   == 0) {
						if (!recursive_set
						    && json_object_get_type(
							       val_i)
							       == json_type_int) {
							ph->recursive =
								json_object_get_int(
									val_i)
								== 1;
							recursive_set = true;
						} else {
							fprintf(stderr,
								"\"recursive\" value not an int\n");
							exit(EXIT_FAILURE);
						}
					} else if (strcmp(key_i,
							  "stop_unchanged")
						   == 0) {
						if (!stop_unchanged_set
						    && json_object_get_type(
							       val_i)
							       == json_type_int) {
							ph->stop_unchanged =
								json_object_get_int(
									val_i);
							stop_unchanged_set =
								true;
						} else {
							fprintf(stderr,
								"\"stop_unchanged\" value not an int\n");
							exit(EXIT_FAILURE);
						}
					} else if (strcmp(key_i,
							  "multithreaded")
						   == 0) {
						if (!multithreaded_set
						    && json_object_get_type(
							       val_i)
							       == json_type_int) {
							ph->multithreaded =
								json_object_get_int(
									val_i);
							multithreaded_set =
								true;
						} else {
							fprintf(stderr,
								"\"multithreaded\" value not an int\n");
							exit(EXIT_FAILURE);
						}
					} else {
						fprintf(stderr,
							"unknowk key \"%s\"\n",
							key_i);
					}
				}
			} else {
				fprintf(stderr,
					"\"graph-parser\" value not an object\n");
				exit(EXIT_FAILURE);
			}
		} else {
			fprintf(stderr, "unknown key \"%s\"", key);
		}
	}
	if (jobj != NULL) {
		json_object_put(jobj);
		/* json_object_object_del(jobj, "");*/
	}
	free(buffer);
	if (completed == 5 && heuristic_set && weights_set && recursive_set
	    && stop_unchanged_set && multithreaded_set) {
		return 0;
	}
	return -1;
}
