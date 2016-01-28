#include <iostream>
#include "parser.h"
#include "utility.h"
#include "centrality.h"
#include "bi_connected_components.h"
#include "graph_manager.h"


void handleSimpleGraph() {
    /* I don't understand the output. Why there are 8 vertices?
        Simple graph
        0
        1
        2
        3
        4
        5
        6
        7
        8
    */
    typedef boost::adjacency_list<> G;
    G a;
    {
        boost::graph_traits<G>::vertex_descriptor v, u, t;
        u = vertex(1, a);
        v = vertex(8, a);
        // t = vertex(5, a);
        add_edge(u, v, a);
        // add_edge(u, t, a);
    }

    std::set<typename boost::graph_traits<G>::vertex_descriptor> av;
    cout << "Simple graph" << endl;
    BGL_FORALL_VERTICES_T(v, a, G) {
        cout << v << endl;
    }
}
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
    outops::operator<<(cout, g);

    // Applying the betweenness centrality
    simpleBetweennessCentrality(g, "json_olsr");

    cout << "Done with Betweenness Centrality" << endl;
}

void handleComplexJsonInput(string filePath) {
    Graph g;
    readComplexJson(filePath, g);
    outops::operator<<(cout, g);

    // Applying the betweenness centrality
    simpleBetweennessCentrality(g, "json_topology");

    cout << "Done with Betweenness Centrality" << endl;
}

void testHeuristic(string filePath) {
    GraphManager gm;
    readEdgeFileGraphManager(filePath, gm);
    BiConnectedComponents bcc(gm);
    cout << "DONE" << endl;
}

void testGraphManager(string filePath) {
    GraphManager gm;
    readEdgeFileGraphManager(filePath, gm);
    cout << gm;

    gm.ResetVerticesAndEdgesIndexMap();
    gm.print_v_index_map();

}

int main(int, char *[]) {
//    string edgeFilePath = "../input/ninux_30_1.edges";
//    testHeuristic(edgeFilePath);
//    handleSimpleInput(edgeFilePath);
//
//    string jsonFilePath = "../input/olsr-netjson.json";
//    handleJsonInput(jsonFilePath);
//
//    string complexJsonFilePath = "../input/jsoninfo_topo.json";
//    handleComplexJsonInput(complexJsonFilePath);

    // string simpleGraphFilePath = "../input/simple.edges";
    // testGraphManager(simpleGraphFilePath);

    string simpleGraphFilePath = "../input/simple.edges";
    testHeuristic(simpleGraphFilePath);

    return 0;
}