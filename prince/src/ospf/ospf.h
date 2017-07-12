#ifndef SRC_OSPF_H_
#define SRC_OSPF_H_

#include "../common.h"
#include "../parser.h"
#include "../socket.h"

/*inehrit methods from here */
#include "../plugin_interface.h"
#define SERVER_READ_BUF_LEN 4096

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

void bird_connect(int *server_fd, char *server_path);

#endif /* SRC_OSPF_H_ */
