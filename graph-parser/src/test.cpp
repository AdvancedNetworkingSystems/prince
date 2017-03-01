#include "graph_parser.h"
#include "parser.h"


int main(int argc, char** argv) {
    int heuristic=atoi(argv[1]);
    char * node_name=argv[2];
    vector<pair<string,double>> map;
    vector<pair<string, int> > degree_map;
    std::ifstream file;
    file.open("input.json");

    GraphManager gm;
    parse_netjson(file,gm);
    if(heuristic==0){
    BetweennessCentrality bc(gm);
    bc.init(gm);
    bc.CalculateBetweennessCentrality();
    gm.get_degrees(degree_map);
	    bc.compose_bc_map(map);
		
	   

	
	}else{

    
    degree_map.clear();
    BetweennessCentralityHeuristic bch(gm);
    bch.init(gm);
    bch.CalculateBetweennessCentrality();
    gm.get_degrees(degree_map);
    bch.compose_bc_map(map);
	/*
    for(auto id_bc_score_pair : map) {
	    cout << id_bc_score_pair.first << "\t" << id_bc_score_pair.second << endl;
    }
	
	*/
}
int i=0;
printf("{\n");
 for(auto id_bc_score_pair : map) {
	cout << id_bc_score_pair.first << ":" << id_bc_score_pair.second ;
	if(i<map.size()-1)
		printf(",");
	printf("\n");
	i++;
}
printf("}\n");
/*
 for(auto id_bc_score_pair : map) {
			if(id_bc_score_pair.first.compare(node_name)==0){
cout << id_bc_score_pair.second << endl;
		}
		   // cout << id_bc_score_pair.first << "\t" << id_bc_score_pair.second << endl;
	    }*/

}
