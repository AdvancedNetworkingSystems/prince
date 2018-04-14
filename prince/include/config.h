#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <errno.h>

#include "prince_handler.h"

int parse_json_config(const char *filepath, prince_handler_t ph);
char *read_file_content(const char *filename);
int read_config_file(prince_handler_t ph, const char *filepath);

#endif /* SRC_CONFIG_H_ */
