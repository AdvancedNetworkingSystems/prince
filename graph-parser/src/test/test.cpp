#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Utility
#include <boost/test/unit_test.hpp>
#include "../graph_parser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace boost;
using namespace boost::unit_test;

#define FILENAME "../../input/olsr-netjson.json"
#define JSONINFO "../../input/jsoninfo_topo.json"

BOOST_AUTO_TEST_CASE(test_c_wrapper){
	FILE *fd = fopen(FILENAME, "r");
	struct stat st;
	fstat(fileno(fd), &st);
	char *buffer = (char*) malloc(st.st_size);
	fread(buffer, st.st_size,1, fd);

	c_graph_parser *gp = new_graph_parser(false, false);
	graph_parser_parse_netjson(gp, buffer);
	graph_parser_calculate_bc(gp);
	map_id_bc_pair map;
	graph_parser_compose_bc_map(gp, &map); //TODO: free
	BOOST_CHECK_EQUAL(map.size, 24);
	for(int i=0;i<map.size ;i++){
		if(strcmp(map.map[i].id, "10.150.25.1")==0){
			BOOST_CHECK_CLOSE(map.map[i].bc, 0.715415, 0.01);
		}
	}
}

BOOST_AUTO_TEST_CASE(degree_test_base){
	ifstream ifs(FILENAME);
	graph_parser gp(1, 0);
	vector<pair<string, int> > map;
	gp._parse_netjson(ifs);
	gp.calculate_bc();
	gp.compose_degree_map(map);
	for(pair<string, int> id_degree_score_pair : map) {
		if(strcmp(id_degree_score_pair.first.c_str(), "10.150.25.1")==0){
			BOOST_CHECK_EQUAL(id_degree_score_pair.second, 5);
		}
	}
	BOOST_CHECK_EQUAL(map.size(), 24);
}


void generic_test_base(bool weight, bool heur){
	ifstream ifs(JSONINFO);
	graph_parser gp(weight, heur);
	vector<pair<string, double> > map;
	gp._parse_jsoninfo(ifs);
	gp.calculate_bc();
	gp.compose_bc_map(map);
	for(auto id_bc_score_pair : map) {
		if(strcmp(id_bc_score_pair.first.c_str(), "10.150.25.1")==0){
			cout << id_bc_score_pair.second << endl;
			BOOST_CHECK_CLOSE(id_bc_score_pair.second, 0.715391, 0.5);
		}
	}
	BOOST_CHECK_EQUAL(map.size(), 24);
}

BOOST_AUTO_TEST_CASE(Test_base){
	//weight =false heuristic=false
	generic_test_base(false, false);

}

BOOST_AUTO_TEST_CASE(test_base_weighted){
	//weight = true
	generic_test_base(true, false);

}

BOOST_AUTO_TEST_CASE(test_heuristic_unweighted){
	//weight = false heurisic=true
	generic_test_base(false, true);

}

BOOST_AUTO_TEST_CASE(test_heuristic_weighted){
	//weight = true heurisic=true
	generic_test_base(true, true);

}
