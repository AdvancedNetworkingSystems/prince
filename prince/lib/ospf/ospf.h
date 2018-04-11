#ifndef SRC_OSPF_H_
#define SRC_OSPF_H_

#include "common.h"
#include "topology_parser.h"
#include "socket.h"

/*inehrit methods from here */
#include "plugin_interface.h"
#define SERVER_READ_BUF_LEN 4096
#define NETJSON_CODE 100
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdint.h>
typedef uint8_t byte;

void bird_connect(routing_plugin *o);
int receive_netjson(routing_plugin *o);

#endif /* SRC_OSPF_H_ */
