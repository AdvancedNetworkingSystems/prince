#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Utility
#include <boost/test/unit_test.hpp>
#include "../graph_parser.h"

using namespace boost;
using namespace boost::unit_test;


BOOST_AUTO_TEST_CASE(Test_library){
	ifstream ifs("olsr-netjson.json");
	graph_parser gp = new graph_parser();
	vector<pair<string, double> > map;
	gp._parse_netjson(ifs);
	gp.compose_bc_map(map);
	BOOST_CHECK_EQUAL(map.size(), 24);

}
