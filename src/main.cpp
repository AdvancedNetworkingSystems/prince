#include <iostream>
#include "parser.h"
#include "utility.h"
#include "centrality.h"


void handleSimpleInput(string filePath) {
    // Read the input.edges
    Graph g;
    readEdgeFile(filePath, g);

    cout << "Finish creating graph" << endl;

    simpleBetweennessCentrality(g, "edge_list");
}

void handleJsonInput(string filePath) {
    Graph g;
    readJson(filePath, g);
    printGraph(g);

    // Applying the betweenness centrality
    simpleBetweennessCentrality(g, "json_olsr");

    cout << "Done with Betweenness Centrality" << endl;
}

void handleComplexJsonInput(string filePath) {
    Graph g;
    readComplexJson(filePath, g);
    printGraph(g);

    // Applying the betweenness centrality
    simpleBetweennessCentrality(g, "json_topology");

    cout << "Done with Betweenness Centrality" << endl;
}


int main(int, char *[]) {
    string edgeFilePath = "../input/ninux_30_1.edges";
    handleSimpleInput(edgeFilePath);

    string jsonFilePath = "../input/olsr-netjson.json";
    handleJsonInput(jsonFilePath);

    string complexJsonFilePath = "../input/jsoninfo_topo.json";
    handleComplexJsonInput(complexJsonFilePath);

    return 0;
}