/*
    Output the number of nodes for each biconnected.
*/
#include <iostream>
#include <vector>
#include <dirent.h>
#include "utility.h"
#include "bi_connected_components.h"
#include "graph_manager.h"

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

void count_bcc(string input_filepath, vector<int>& number_of_nodes_in_bcc, bool is_weighted_graph) {
    streambuf* orig_buf = cout.rdbuf(); // get underlying buffer
    cout.rdbuf(0); // set null

    GraphManager gm(is_weighted_graph);
    readEdgeFileGraphManager(input_filepath, gm);
    BiConnectedComponents bcc = BiConnectedComponents(gm);
    bcc.FindBiConnectedComponents();

    // Counting number of nodes in each bi-connected component
    for (int i = 0; i < bcc.num_of_bcc(); ++i) {
        int num_of_vertices = bcc.BCCs[i].num_of_vertices();
        number_of_nodes_in_bcc.push_back(num_of_vertices);
    }
    cout.rdbuf(orig_buf);
}

void write_counting_result(string output_filepath, string filename, const vector<int>& number_of_nodes_in_bcc) {
    ofstream out_file(output_filepath.c_str(), std::ofstream::out | std::ofstream::app);

    // Extract information from file name
    string graph_type = filename.substr(0, 2);

    string::size_type sep_pos = filename.find('_');
    string number_of_nodes = filename.substr(2, sep_pos - 2);

    string topo_type = graph_type + number_of_nodes;

    // Number of bi-connected components
    int num_bccs = number_of_nodes_in_bcc.size();

    // String containing the number of nodes
    std::stringstream result;
    std::copy(number_of_nodes_in_bcc.begin(), number_of_nodes_in_bcc.end(), std::ostream_iterator<int>(result, " "));
    string result_str = result.str();

    string separator = ", ";

    if (out_file.is_open()) {
        out_file << filename << separator;
        out_file << topo_type << separator;
        out_file << num_bccs << separator;
        out_file << result_str << endl;
    }
    out_file.close();
}

int main(int argc, char * argv[]) {
    if (argc < 2) {
        cout << "Provide the input directory\n";
        exit(1);
    }

    string input_dir = string(argv[1]);

    string output_filepath = "../output/counting_bcc.out";
    if (argc >= 3) {
        output_filepath = string(argv[2]);
    }

    bool is_weighted_graph = false;
    if (argc >= 4) {
        if (string(argv[3]).compare("false") != 0) {
            is_weighted_graph = true;
        }
    }


    // Find all the files
    vector<string> files;
    get_all_files_in_directory(input_dir, files, "edges");

    for (string file : files) {
        string input_filepath = input_dir + "/" + file;
        vector<int> number_of_nodes_in_bcc;
        count_bcc(input_filepath, number_of_nodes_in_bcc, is_weighted_graph);

        write_counting_result(output_filepath, file, number_of_nodes_in_bcc);
    }
}