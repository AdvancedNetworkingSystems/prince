#include "graph_parser.h"


graph_parser::graph_parser(bool weight, bool _heuristic): gm(weight), heuristic(_heuristic){
#ifdef LOG
    boost::log::add_console_log(std::cout, boost::log::keywords::format = ">> %Message%");
	cout << "LOGGING ENABLED" << endl;
#endif
	if(heuristic) bci = new BetweennessCentralityHeuristic();
	else bci = new BetweennessCentrality();
};


graph_parser::~graph_parser(){
	delete bci;
}
/**
 * Parse simple graph from struct topology
 * @param struct topology *topo   struct containing an adjacency list representation of the topology
 */
void
graph_parser::_parse_simplegraph(struct topology *topo){
  parse_simplegraph(topo, gm);
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
	  graph_parser_parse_simplegraph(c_graph_parser* v, struct topology *topo){
		  try{
			  graph_parser *vc = (graph_parser*)v;
			  vc->_parse_simplegraph(topo);
		  }catch(...){
			  cout << "Error parsing Netjson";
		  }

	  }

	  void
	  graph_parser_calculate_bc(c_graph_parser* v){
		  graph_parser *vc = (graph_parser*)v;
		  try{
			  vc->calculate_bc();
		  }catch(...){
			  cout << "error calculating bc";
		  }
	  }

	  int
	  graph_parser_compose_degree_bc_map(c_graph_parser* v, map_id_degree_bc * map){

		try{
			graph_parser *vc = (graph_parser*)v;
			int i=0;
			vector<pair<string, int> > cpp_degree_map;
			vector<pair<string, double> > cpp_bc_map;
			vc->compose_degree_map(cpp_degree_map);
			vc->compose_bc_map(cpp_bc_map);
			map->size=cpp_degree_map.size();
			map->map = (id_degree_bc *) malloc(sizeof(_id_degree_bc)*map->size);
			map->n_edges = vc->get_n_edges();

			for(i=0;i<map->size;i++){
				if(cpp_degree_map[i].first.compare(cpp_bc_map[i].first)==0){
					map->map[i].id = strdup(cpp_degree_map[i].first.c_str());
					map->map[i].bc = cpp_bc_map[i].second;
					map->map[i].degree = cpp_degree_map[i].second;
				}else{
					throw 0;
				}
			}
			return 1;


		  }catch(...){
			  cout << "error composing bc degree map";
			  return 0;
		  }
		  return 0;

	  }


	  void
	  delete_graph_parser(c_graph_parser* v) {
		graph_parser *vc = (graph_parser*)v;
		delete vc;
	  }
}

