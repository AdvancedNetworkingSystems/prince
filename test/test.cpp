#include "python2.7/Python.h"
#include "../graph-parser_c/src/graph_parser.h"
#include "../prince/src/parser.h"
//#include "../graph-parser/src/graph_parser.h"
#include "python2.7/Python.h" 

c_graph_parser* cg;
extern "C" {
	void init(int h){
	cg=new_graph_parser(1, h);
	}
	void parse(char * g){
	struct topology * t=parse_netjson(g);
	graph_parser_parse_simplegraph(cg,t);
	}
	void compute(){
	graph_parser_calculate_bc(cg);
	}
	PyObject*  get_res(){
		map_id_degree_bc map;
		graph_parser_compose_degree_bc_map(cg, &map);
		int i;	
		PyObject*  d=PyDict_New();
		for(i=0;i<map.size;i++){
		PyDict_SetItem(d,PyInt_FromString(map.map[i].id,0,10),PyFloat_FromDouble(map.map[i].bc));
		}
	return d;
	}
	void destroy(){
		delete_graph_parser(cg);
	}
}
