/*
 * main.c
 *
 *  Created on: 11 mag 2016
 *      Author: gabriel
 */

#include <stdio.h>
#include "../graph_parser.h"
#include  <unistd.h>

int main(){
	FILE * file;
	struct stat stat;
	file = fopen("../../input/test/olsr-netjson.json", "r");
	c_graph_parser gp;
	gp = new_graph_parser(1,0);

	//graph_parser_parse_netjson(gp, )
}

