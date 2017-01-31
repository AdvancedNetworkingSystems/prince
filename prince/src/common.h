/*
Copyright (c) 2016 Gabriele Gemmi <gabriel@autistici.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/*
 * common.h
 *
 *  Created on: 25 mag 2016
 *      Author: gabriel
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_


/*INCLUDES */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../../graph-parser_c/src/graph_parser.h"

/*DEFINES*/
#define true 1
#define false 0
#define LINE_SIZE 64


struct timers{
	double h_timer;
	double tc_timer;
	double exec_time;
};


typedef struct
routing_plugin_{
	char *recv_buffer;
	char *self_id;
	char *host;
	short port;
	int json_type;
	c_graph_parser *gp;

}routing_plugin;



#endif /* SRC_COMMON_H_ */
