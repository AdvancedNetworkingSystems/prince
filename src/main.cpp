#include <cstdlib>
#include <iostream>
#include <string>
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

void default_run(string filepath, int input_type, bool is_weighted_graph, bool endpoints_inclusion) {
    // input_files = [(filepath, input_type)]
    // input_type = 1 ==> edges file
    // input_type = 2 ==> simple json file
    // input_type = 3 ==> complex json file

    GraphManager gm(is_weighted_graph);

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

    BiConnectedComponents bcc = BiConnectedComponents(gm);
    bcc.run();

    // Calculate Betweenness Centrality
    cout << "Calculate Betweenness Centrality\n";
    bcc.CalculateBetweennessCentrality(endpoints_inclusion);
    // bcc.print();

    string filename;
    string ext;
    helper::get_file_name_and_extension(filepath, filename, ext);

    string out_filepath = "../output/" + filename;
    if (is_weighted_graph) {
        out_filepath += "_weighted.out";
    }
    else {
        out_filepath += "_unweighted.out";
    }
    bcc.write_all_betweenness_centrality(out_filepath);
}

void old_main_code() {
    string simpleGraphfilepath = "../input/simple.edges";
    testGraphManager(simpleGraphfilepath);

    testHeuristic(simpleGraphfilepath);
}

int main(int argc, char * argv[]) {
    // Example: ./main simple.edges 1 true true

    bool is_weighted_graph = true;
    bool endpoints_inclusion = true;
    string filepath;
    int input_type;
    string argument;

    if (argc >= 3) {
        filepath = string(argv[1]);
        input_type = atoi(argv[2]);
    }
    if (argc >= 4) {
        argument = string(argv[3]);
        if (argument.compare("false") == 0) {
            is_weighted_graph = false;
        }

        if (argc > 4) {
           argument = string(argv[4]);
            if (argument.compare("false") == 0) {
                endpoints_inclusion = false;
            }
        }
    }

    default_run(filepath, input_type, is_weighted_graph, endpoints_inclusion);

    cout << "is weighted graph  = " << is_weighted_graph << endl;
    return 0;
}