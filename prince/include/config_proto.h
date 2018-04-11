#ifndef SRC_CONFIG_PROTO_H_
#define SRC_CONFIG_PROTO_H_

#include <errno.h>

typedef struct proto_config * proto_config_t;

#include "config.h"

#define PROTO_CONFIG_SIZE    sizeof(struct proto_config)
#define INVALID_PROTO_CONFIG NULL

struct proto_config {
        char *proto;
        int  json_type;
        int  port;
        int  refresh;
        int  sleep_onfail;
        int  timer_port;
};

proto_config_t new_proto_config(void);
void           free_proto_config(proto_config_t conf);
int            load_proto_config(const char *filepath, proto_config_t conf);

#endif /* SRC_CONFIG_PROTO_H */
