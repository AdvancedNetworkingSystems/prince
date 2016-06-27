#include "graph_parser.h"


graph_parser::graph_parser(bool weight, bool _heuristic): gm(weight), heuristic(_heuristic){
	if(heuristic) bci = new BetweennessCentralityHeuristic();
	else bci = new BetweennessCentrality();
};

/**
 * Parse netjson from istream
 * @param &istream  stream containing the buffered netjson
 */
void
graph_parser::_parse_netjson(std::basic_istream<char> &istream){
  parse_netjson(istream, gm);
}

/**
 * Parse jsoninfo from istream
 * @param &istream  stream containing the buffered jsoninfo
 */
void
graph_parser::_parse_jsoninfo(std::basic_istream<char> &istream){
  parse_jsoninfo(istream, gm);
}

/**
 * Calculate the BC
 */
void
graph_parser::calculate_bc(){
	bci->init(gm);
	bci->CalculateBetweennessCentrality();

}

/**
 * Compose the BC values in a map
 * @param &map  reference to the map where BC values will be stored
 */
void
graph_parser::compose_bc_map(vector<pair<string, double> > &map){
  bci->compose_bc_map(map);
}

/**
 * Compose the vertexes degrees' values in a map
 * @param &map  reference to the map where Degrees values will be stored
 */
void
graph_parser::compose_degree_map(vector<pair<string, int> > &map){
 gm.get_degrees(map);
}

/**
 * Return the number of the edges of the graph
 */
int
graph_parser::get_n_edges(){
  return num_edges(gm.g_);

}


extern "C" {
	c_graph_parser*
	new_graph_parser(int weight, int heuristic){
		graph_parser *v = (graph_parser*)new graph_parser(weight, heuristic);
		return (c_graph_parser *)v;
	  }

	  void
	  graph_parser_parse_netjson(c_graph_parser* v, char *json){
		  try{
			  graph_parser *vc = (graph_parser*)v;
			  std::istringstream ss(json);
			  vc->_parse_netjson(ss);
		  }catch(...){
			  cout << "Error parsing jsoninfo";
		  }

	  }

	  void
	  graph_parser_parse_jsoninfo(c_graph_parser* v, char *json){
		  try{
			  graph_parser *vc = (graph_parser*)v;
			  std:istringstream ss(json);
			  vc->_parse_jsoninfo(ss);
		  }catch(...){
			  cout << "Error parsing Netjson";
		  }

	  }

	  void
	  graph_parser_calculate_bc(c_graph_parser* v){
		  //TODO: Manage exceptions
		  graph_parser *vc = (graph_parser*)v;
		  try{
			  vc->calculate_bc();
		  }catch(...){
			  cout << "error calculating bc";
		  }
	  }

	  void
	  graph_parser_compose_bc_map(c_graph_parser* v, map_id_bc_pair * map){
		graph_parser *vc = (graph_parser*)v;
		vector<pair<string, double> > cppmap;
		try{
			vc->compose_bc_map(cppmap);
		}catch(...){
			cout << "error composing bc map";
		}
		int i=0;
		map->size=cppmap.size();
		map->map = new id_bc_pair[map->size];
		for(pair<string, double> item: cppmap){
			map->map[i].id = new char[strlen(item.first.c_str())];
			strcpy(map->map[i].id, item.first.c_str());
			map->map[i].bc = item.second;
			i++;
		}
	  }

	  void
	  graph_parser_compose_degree_map(c_graph_parser* v, map_id_degree_pair * map){
		graph_parser *vc = (graph_parser*)v;
		vector<pair<string, int> > cppmap;
		try{
			vc->compose_degree_map(cppmap);
		}catch(...){
			cout << "error composing degree map";
		}
		int i=0;
		map->size=cppmap.size();
		map->n_edges = vc->get_n_edges();
		map->map = new id_degree_pair[map->size];
		for(pair<string, int> item: cppmap){
			map->map[i].id = new char[strlen(item.first.c_str())];
			strcpy(map->map[i].id, item.first.c_str());
			map->map[i].degree = item.second;
			i++;
		}
	  }

	  void
	  delete_graph_parser(c_graph_parser* v) {
		graph_parser *vc = (graph_parser*)v;
		delete vc;
	  }
}

