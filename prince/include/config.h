#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include "prince.h"

int    parse_json_config(char *filepath, prince_handler_t ph);
char * read_file_content(char *filename);
int    read_config_file(prince_handler_t ph, char *filepath);

#endif
