#ifndef SRC_OSPF_H_
#define SRC_OSPF_H_

#include "../../src/common.h"
#include "../../src/parser.h"
#include "../../src/socket.h"

/*inehrit methods from here */
#include "../../src/plugin_interface.h"
#define SERVER_READ_BUF_LEN 4096
#define NETJSON_CODE 100
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
typedef uint8_t byte;

void bird_connect(routing_plugin *o);
int receive_netjson(routing_plugin *o);

#endif /* SRC_OSPF_H_ */
