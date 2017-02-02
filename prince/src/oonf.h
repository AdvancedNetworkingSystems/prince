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
 * olsr.h
 *
 *  Created on: 25 mag 2016
 *      Author: gabriel
 */

#ifndef SRC_OONF_H_
#define SRC_OONF_H_

#include "common.h"
#include "socket.h"
#include "parser.h"

#ifndef unique
/*inehrit methods from here */
#include "plugin_interface.h"
#else
routing_plugin* new_plugin_oonf(char* host, int port, c_graph_parser *gp, int json_type);
int get_topology_oonf(routing_plugin *o);
int push_timers_oonf(routing_plugin *o, struct timers t);
void delete_plugin_oonf(routing_plugin* o);
#endif


#endif /* SRC_OONF_H_ */
