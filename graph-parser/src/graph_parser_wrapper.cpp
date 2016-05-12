/*
 * graph_parser_wrapper.cpp
 *
 *  Created on: 10 mag 2016
 *      Author: gabriel
 */


#include "graph_parser_wrapper.h"



extern "C" {
		graph_parser* new_graph_parser() {
                return new graph_parser();
        }

        void graph_parser_parse_netjson(graph_parser* v, char *json){
            	membuf sbuf(json, json + sizeof(json));
                std::istream in(&sbuf);
                v->_parse_netjson(in);
        }

        void graph_parser_calculate_bc(graph_parser* v){
        	v->calculate_bc();
        }

        void graph_parser_compose_bc_map(graph_parser* v, char *map){
        	vector<pair<string, double> > cppmap;
        	//v->compose_bc_map(cppmap);
        	map=cppmap.data();
        }


        void deleteMyClass(graph_parser* v) {
                delete v;
        }
}

