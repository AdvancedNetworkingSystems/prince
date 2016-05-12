#include "graph_parser.h"
#include "parser.h"


  graph_parser::graph_parser(bool weight, bool _heuristic): heuristic(_heuristic), gm(weight){};

  void graph_parser::_parse_netjson(std::basic_istream<char> &istream){
	  parse_netjson(istream, gm);
  }
  void graph_parser::calculate_bc(){
	  bcc.init(gm);
	  bc.init(gm);
	  if(heuristic){
		  bcc.CalculateBetweennessCentrality();
	  }else{
		  bc.CalculateBetweennessCentrality();
	  }
  }

  void graph_parser::compose_bc_map(vector<pair<string, double> > &map){
  	  if(heuristic){
		  bcc.compose_bc_map(map);

  	  }else{
  		  bc.compose_bc_map(map);
  	  }
    }

