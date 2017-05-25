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


static void bird_connect(char *server_path );
static void server_send(char *cmd);
static void server_got_reply(char *x);
