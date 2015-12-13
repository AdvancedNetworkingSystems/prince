//
// Created by quynh on 12/13/15.
//

#include "parser.h"

vector<graphDataType> readEdgeFile(string filePath) {
    ifstream inFile(filePath);

    // Reading to vector<graphDataType>
    vector<graphDataType> contents;
    vector<string> strs;
    graphDataType graph_strs;
    for (string line; getline(inFile, line); /**/) {
        boost::split(strs, line, boost::is_any_of(" "));

        // Cast vector<string> to array<string, 3>
        // TODO: this is really crude way to do it.
        // TODO: how to copy some element of vector to array
        vector<string>::iterator i = strs.begin();
        for (int i = 0; i < strs.size(); ++i) {
            graph_strs[i] = strs[i];
        }
        contents.push_back(graph_strs);
    }
    inFile.close();

    return contents;
}
