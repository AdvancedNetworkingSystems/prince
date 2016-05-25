/*
 * common.h
 *
 *  Created on: 25 mag 2016
 *      Author: gabriel
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_


//INCLUDES
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "../../graph-parser/src/graph_parser.h"

//DEFINES
#define true 1
#define false 0
#define LINE_SIZE 64
#define BUFFER_SIZE 1024
#define SIZE_TO_READ 1024

struct timers{
	double h_timer;
	double tc_timer;
};

//FUNCTIONS DECLARATION
int _create_socket(char* destinazione, int porta);
int _http_receive(int sd, char **buffer);
int _receive_data(int sd, char **finalBuffer);
int _check_header_clen(char *header, char *body);


#endif /* SRC_COMMON_H_ */
