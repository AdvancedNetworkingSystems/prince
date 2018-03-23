#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

typedef struct proto_config * proto_config_t;

#include "prince.h"
#include "prince_handler.h"


struct proto_config {
        int json_type;
        int port;
        int refresh;
        int sleep_onfail;
        int timer_port;
};

int    parse_json_config(char *filepath, prince_handler_t ph);
char * read_file_content(char *filename);
int    read_config_file(prince_handler_t ph, char *filepath);

#endif /* SRC_CONFIG_H_ */
