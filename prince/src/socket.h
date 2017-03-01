#ifndef SRC_SOCKET_H
#define SRC_SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "common.h"

#define BUFFER_SIZE 1024
#define SIZE_TO_READ 1024


/*FUNCTIONS DECLARATION*/
int _create_socket(char* destinazione, int porta);
int _http_receive(int sd, char **buffer);
int _telnet_receive(int sd, char **finalBuffer);
int _receive_data(int sd, char **finalBuffer);
int check_header_clen(char *header, char *body);




#endif
