//
// Created by quynh on 12/15/15.
//

#include "utility.h"


void printGraph(Graph &g) {
    typename boost::graph_traits<Graph>::out_edge_iterator out_i, out_ie;
    Viter v_i, v_ie;

    Vertex v;
    Edge e;

    boost::tie(v_i, v_ie) = boost::vertices(g);
    for (v_i; v_i != v_ie; ++v_i) {
        v = *v_i;
        boost::tie(out_i, out_ie) = boost::out_edges(v, g);

        for (out_i; out_i != out_ie; ++out_i) {
            e = *out_i;
            Vertex src = boost::source(e, g);
            Vertex targ = boost::target(e, g);
            std::cout << "(" << g[src].id << ","
            << g[targ].id << ") ";
        }
        cout << endl;
    }
}