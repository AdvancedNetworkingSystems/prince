/*
 * main.cpp
 *
 *  Created on: 11 mag 2016
 *      Author: gabriel
 */
#include <cstdlib>
#include <iostream>
#include <string>
#include "../graph_parser.h"


int main(int argc, char * argv[]) {

	std::ifstream file;
	file.open("test/olsr-netjson.json");

	graph_parser gp(0,0);
	gp._parse_netjson(file);
	gp.calculate_bc();

	vector<pair<string,double>> map;
	gp.compose_bc_map(map);
	for(auto id_bc_score_pair : map) {
		cout << id_bc_score_pair.first << "\t" << id_bc_score_pair.second << endl;
	}
    return 0;
}


