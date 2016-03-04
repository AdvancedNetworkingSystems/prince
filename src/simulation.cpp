#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <vector>
#include "bi_connected_components.h"
#include "centrality.h"
#include "graph_manager.h"
#include "parser.h"
using namespace std;

void get_all_files_in_directory(string dir_path, vector<string>& files, string with_extension) {
    DIR*    dir;
    dirent* pdir;

    dir = opendir(dir_path.c_str());

    while (pdir = readdir(dir)) {
        string filename = pdir->d_name;
        string::size_type idx;
        idx = filename.find('.');
        string extension = "";
        if (idx != string::npos) {
            extension = filename.substr(idx + 1);
        }

        if (extension.compare(with_extension) == 0) {
            files.push_back(filename);
            cout << filename << endl;
        }
    }
}

void calculate_brandes_bc(const GraphManager& gm, bool targets_inclusion) {
    CentralityVec v_centrality_vec = CentralityVec(boost::num_vertices(gm.g_));
    CentralityPMap v_centrality_pmap = CentralityPMap(v_centrality_vec.begin(), gm.v_index_pmap());;

    if (gm.weighted_graph()) { // calculate BC for weighted graph
        cout << "======= BCC - BC for weighted graph ======\n";

        typedef map<Edge, double> EdgeWeightStdMap;
        typedef boost::associative_property_map<EdgeIndexStdMap> EdgeWeightPMap;
        EdgeIndexStdMap edge_weight_std_map;
        EdgeWeightPMap edge_weight_pmap = EdgeWeightPMap(edge_weight_std_map);


        BGL_FORALL_EDGES(edge, gm.g_, Graph) {
            edge_weight_std_map[edge] = gm.g_[edge].cost;
        }
        boost::brandes_betweenness_centrality_targets_inclusion(gm.g_,
            targets_inclusion,
            boost::centrality_map(
                v_centrality_pmap).vertex_index_map(
                gm.v_index_pmap()).weight_map(
                edge_weight_pmap)
        );
    }
    else { // for unweighted graph
        boost::brandes_betweenness_centrality_targets_inclusion(gm.g_,
            targets_inclusion,
            boost::centrality_map(
                v_centrality_pmap).vertex_index_map(
                gm.v_index_pmap())
        );
    }

    boost::relative_betweenness_centrality(gm.g_, v_centrality_pmap);
}

void run_simulation_for_a_graph(string input_path, double& elapsed_secs_bc, double& elapsed_secs_hbc, int number_of_experiments, bool is_weighted_graph) {
    /* Returns the running time for Brandes BC and HBC
    */
    clock_t begin, end;

    // Disable the console output: http://stackoverflow.com/questions/30184998/how-to-disable-cout-output-in-the-runtime
    streambuf* orig_buf = cout.rdbuf(); // get underlying buffer
    cout.rdbuf(0); // set null

    GraphManager gm(is_weighted_graph);
    cout << input_path << endl;
    readEdgeFileGraphManager(input_path, gm);

    // For Brandes BC
    begin = clock();
    bool targets_inclusion = true;
    calculate_brandes_bc(gm, targets_inclusion);
    end = clock();
    elapsed_secs_bc = double(end - begin) / CLOCKS_PER_SEC;

    // For HBC
    begin = clock();
    BiConnectedComponents bcc = BiConnectedComponents(gm);
    bcc.run();
    end = clock();
    elapsed_secs_hbc = double(end - begin) / CLOCKS_PER_SEC;

    // Restore the console output
    cout.rdbuf(orig_buf);

    // time_log = make_tuple(elapsed_secs_bc, elapsed_secs_hbc);
}

void write_simulation_result(string out_file_path, string filename, int run_index, double bc_time, double hbc_time) {
    ofstream out_file(out_file_path.c_str(), std::ofstream::out | std::ofstream::app);

    // Extract information from file name
    string graph_type = filename.substr(0, 2);

    string::size_type sep_pos = filename.find('_');
    string number_of_nodes = filename.substr(2, sep_pos - 2);

    string separator = "\t";
    if (out_file.is_open()) {
        out_file << filename << separator;
        out_file << graph_type << separator;
        out_file << number_of_nodes << separator;
        out_file << run_index << separator;
        out_file << bc_time << separator;
        out_file << hbc_time << separator << endl;
    }
    out_file.close();
}

int main(int argc, char * argv[]) {
    /*
    Input: the folder with all the *.edges graph file

    For each graph, run the simulation <num_of_times> times
    Write the output into one csv file

    filename | graph type | number of nodes | id of the run | Brandes | HBC
    */

    // Handle command-line arguments
    if (argc < 2) {
        cout << "Provide the input directory\n";
        exit(1);
    }

    string input_dir = string(argv[1]);

    int number_of_experiments = 10; // default
    if (argc >= 3) {
        number_of_experiments = atoi(argv[2]);
    }

    string output_file_path = "../output/simulation.out";
    if (argc >= 4) {
        output_file_path = string(argv[3]);
    }

    bool is_weighted_graph = true;
    if (argc >= 5) {
        if (string(argv[4]).compare("false") == 0) {
            is_weighted_graph = false;
        }
    }

    // Find all the files
    vector<string> files;
    get_all_files_in_directory(input_dir, files, "edges");

    for (string file : files) {
        cout << "Running for file " << file << endl;
        for (int run_index = 0; run_index < number_of_experiments; ++run_index) {
            double bc_time;
            double hbc_time;
            string input_path = input_dir + "/" + file;
            run_simulation_for_a_graph(input_path, bc_time, hbc_time, number_of_experiments, is_weighted_graph);
            cout << "    " << run_index << ": " << bc_time << " | " << hbc_time << endl;
            write_simulation_result(output_file_path, file, run_index, bc_time, hbc_time);
        }
    }
}


