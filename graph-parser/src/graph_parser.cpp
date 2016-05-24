#include "graph_parser.h"


  graph_parser::graph_parser(bool weight, bool _heuristic): gm(weight), heuristic(_heuristic){};

  void graph_parser::_parse_netjson(std::basic_istream<char> &istream){
	  parse_netjson(istream, gm);
  }

  void graph_parser::_parse_jsoninfo(std::basic_istream<char> &istream){
	  parse_jsoninfo(istream, gm);
  }
  void graph_parser::calculate_bc(){
	  if(heuristic){
		  bcc.init(gm);
		  bcc.CalculateBetweennessCentrality();
	  }else{
		  bc.init(gm);
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




  struct membuf : std::streambuf
  {
      membuf(char* begin, char* end) {
          this->setg(begin, begin, end);
      }
  };



  extern "C" {
  		c_graph_parser* new_graph_parser(int weight, int heuristic){
      		graph_parser *v = (graph_parser*)new graph_parser(weight, heuristic);
      		return (c_graph_parser *)v;
          }

          void graph_parser_parse_netjson(c_graph_parser* v, char *json){
          		graph_parser *vc = (graph_parser*)v;
                std::istringstream ss(json);
                vc->_parse_netjson(ss);
          }

          void graph_parser_parse_jsoninfo(c_graph_parser* v, char *json){
        	  graph_parser *vc = (graph_parser*)v;
        	  std:istringstream ss(json);
        	  vc->_parse_jsoninfo(ss);
          }

          void graph_parser_calculate_bc(c_graph_parser* v){
      		graph_parser *vc = (graph_parser*)v;
          	vc->calculate_bc();
          }

          void graph_parser_compose_bc_map(c_graph_parser* v, map_id_bc_pair * map){
      		graph_parser *vc = (graph_parser*)v;
          	vector<pair<string, double> > cppmap;
          	vc->compose_bc_map(cppmap);

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



          void delete_my_class(c_graph_parser* v) {
      		graph_parser *vc = (graph_parser*)v;
          	delete vc;
          }
  }

