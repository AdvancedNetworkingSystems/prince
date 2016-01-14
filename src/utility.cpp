//
// Created by quynh on 12/15/15.
//

#include "utility.h"
using namespace boost;

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

namespace outops {
    std::ostream& operator<<(std::ostream& os, const Graph& g)
    {

        os <<   "Graph has: \n"
                "---------- " << boost::num_vertices(g) << " vertices\n"
                "---------- " << boost::num_edges(g) << " edges\n";

        std::set<std::string> verticesSet;
        BGL_FORALL_VERTICES_T(v, g, Graph) verticesSet.insert(g[v].id);

        std::vector<std::string> edgesVec;
        std::vector<double> costsVec;
        BGL_FORALL_EDGES_T(e, g, Graph) {
            std::string s = "(" + g[e.m_source].id + ", " + g[e.m_target].id + ") - " + std::to_string(g[e].cost);
            edgesVec.push_back(s);
            costsVec.push_back(g[e].cost);
        }

        using namespace boost::spirit::karma;
        os <<   "Vertices:\n"
                "  ";
        os << format("(" << auto_ % ", " << ") ", verticesSet);
        os << "\n";

        os <<   "Edges:\n";
        os << format("  " << (auto_ % "\n  ") << eol, edgesVec);
        os << "\n";

        return os;
    }

    std::ostream& operator<<(std::ostream& os, const std::set<std::string>& s) {
        /* can't make it work with a generic function
        ** std::ostream& opeartor<<(std::ostream& os, const Container<std::string>& s)
        */
        using namespace boost::spirit::karma;
        os << format("( " << (auto_ % "\n  ") << ")", s);
    }

}

namespace outopserror {
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::set<T>& s) {
        /* can't make it work with a generic function
        ** std::ostream& opeartor<<(std::ostream& os, const Container<std::string>& s)
        */
        using namespace boost::spirit::karma;
        os << format("( " << (auto_ % "\n  ") << ")", s);
    }
}

namespace graphext {
    void id_of_vertices(const Graph& g, std::set<std::string>& r) {
        BGL_FORALL_VERTICES_T(v, g, Graph) {
            r.insert(g[v].id);
        }
    }

    // template <typename Container>
    // void id_of_vertices(const Graph& g, const Container& container, std::set<std::string>& r) {
    void id_of_vertices(const Graph& g, const VertexVec& container, std::set<std::string>& r) {
        /*
        ** Find id for a vec
        */
        for (VertexVec::const_iterator ci = container.begin(); ci != container.end(); ++ci) {
            r.insert(g[*ci].id);
        }
    }
}