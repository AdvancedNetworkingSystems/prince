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

void default_run(bool is_weighted_graph, bool targets_inclusion) {
    // input_files = [(filepath, input_type)]
    // input_type = 1 ==> edges file
    // input_type = 2 ==> simple json file
    // input_type = 3 ==> complex json file

    std::list< std::tuple<string, int> > input_files;
    input_files.push_back(std::make_tuple("../input/simple.edges", 1));
    input_files.push_back(std::make_tuple("../input/ninux_unweighted_connected.edges", 1));
    input_files.push_back(std::make_tuple("../input/ninux_30_1.edges", 1));
    input_files.push_back(std::make_tuple("../input/olsr-netjson.json", 2));
    input_files.push_back(std::make_tuple("../input/jsoninfo_topo.json", 3));

    for (auto input : input_files) {
        string filepath = std::get<0>(input);
        int input_type = std::get<1>(input);

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
        bcc.CalculateBetweennessCentrality(targets_inclusion);
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
}

void old_main_code() {
    string simpleGraphfilepath = "../input/simple.edges";
    testGraphManager(simpleGraphfilepath);

    testHeuristic(simpleGraphfilepath);
}

int main(int argc, char * argv[]) {
    bool is_weighted_graph = true;
    bool targets_inclusion = true;

    if (argc >= 2) {
        string argument(argv[1]);
        if (argument.compare("false") == 0) {
            is_weighted_graph = false;
        }

        if (argc > 2) {
           argument = string(argv[2]);
            if (argument.compare("false") == 0) {
                targets_inclusion = false;
            }
        }
    }

    default_run(is_weighted_graph, targets_inclusion);

    cout << "is weighted graph  = " << is_weighted_graph << endl;
    return 0;
}