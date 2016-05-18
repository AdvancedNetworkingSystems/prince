/*
 * prince.h
 *
 *  Created on: 12 mag 2016
 *      Author: gabriel
 */

#ifndef SRC_PRINCE_H_
#define SRC_PRINCE_H_

#include "../../graph-parser/src/graph_parser.h"

#include <sys/types.h>
#include <sys/socket.h>
//"in" per "sockaddr_in"
#include <netinet/in.h>
//"netdb" per "gethostbyname"
#include <netdb.h>
#include <unistd.h>
#include <strings.h>


#define SIZE_TO_READ 4096
#define true 1
#define false 0



int main();
int receive_data(int sd, char **finalBuffer);
void close_socket(int sock);
int create_socket(char* Destinazione, int Porta);
int check_header_clen(char *header,char *body);

#endif /* SRC_PRINCE_H_ */

