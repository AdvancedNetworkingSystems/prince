#include <iostream>
#include "parser.h"
#include "utility.h"
#include "centrality.h"
#include "bi_connected_components.h"
#include "graph_manager.h"

void testHeuristic(string filepath) {
    GraphManager gm;
    readEdgeFileGraphManager(filepath, gm);
    BiConnectedComponents bcc = BiConnectedComponents(gm);
    bcc.run();
    bcc.print();

    bcc.write_all_betweenness_centrality("../output/simple.edges");
    cout << "DONE" << endl;
}

void testGraphManager(string filepath) {
    GraphManager gm;
    readEdgeFileGraphManager(filepath, gm);
    gm.print();
}

void default_run() {
    // input_files = [(filepath, input_type)]
    // input_type = 1 ==> edges file
    // input_type = 2 ==> simple json file
    // input_type = 3 ==> complex json file

    std::list< std::tuple<string, int> > input_files;
    // input_files.push_back(std::make_tuple("../input/ninux_unweighted_connected.edges", 1));
    // input_files.push_back(std::make_tuple("../input/simple.edges", 1));
    // input_files.push_back(std::make_tuple("../input/ninux_30_1.edges", 1));
    // input_files.push_back(std::make_tuple("../input/olsr-netjson.json", 2));
    input_files.push_back(std::make_tuple("../input/jsoninfo_topo.json", 3));

    for (auto input : input_files) {
        string filepath = std::get<0>(input);
        int input_type = std::get<1>(input);
        GraphManager gm;
        if (input_type == 1) {
            readEdgeFileGraphManager(filepath, gm);
        }
        else if (input_type == 2) {
            readJsonGraphManager(filepath, gm);
        }
        else if (input_type == 3) {
            readComplexJsonGraphManager(filepath, gm);
        }

        gm.print();

        BiConnectedComponents bcc = BiConnectedComponents(gm, false);
        bcc.run();
        // bcc.print();
        string filename = helper::get_file_name(filepath);
        string out_filepath = "../output/" + filename + ".out";;
        bcc.write_all_betweenness_centrality(out_filepath);
    }
}

void old_main_code() {
    string simpleGraphfilepath = "../input/simple.edges";
    testGraphManager(simpleGraphfilepath);

    testHeuristic(simpleGraphfilepath);
}

int main(int, char *[]) {
    default_run();

    return 0;
}