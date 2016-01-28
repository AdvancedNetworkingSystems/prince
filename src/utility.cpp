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

/* outops namespace */
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

    std::ostream& operator<<(std::ostream& os, std::pair<const Graph&, const VertexIndexMap&> p) {
        // ERROR: wrong output.
        // I think it's because of copy constructor.
        // Check out shell_output/w14

        // Calling example:
        // outops::operator<<(cout, std::pair<const Graph&, const VertexIndexMap&>(g_, v_index_map_));
        Graph g = p.first;
        VertexIndexMap v_index_map = p.second;

        std::list<std::string> outputs;
        BGL_FORALL_VERTICES_T(v, g, Graph) {
            int index = boost::get(v_index_map, v);
            cout << g[v].id << ": " << v << endl;
            string vertex_id = g[v].id;
            outputs.push_back(vertex_id + ": " + std::to_string(index));
        }

        using namespace boost::spirit::karma;
        os << "Vertex Index Map:\n";
        os << "[\n";
        os << format("  " << (auto_ % "\n  ") << "]\n", outputs);

        return os;
    }

    std::ostream& operator<<(std::ostream& os, const std::map<string, int>& m)
    {
        // similar to printhelper::for_map()
        os << "cout << std::map\n";
        std::map<string, int>::const_iterator iter;
        for (iter = m.begin(); iter != m.end(); ++iter) {
            os << (*iter).first << ": " << (*iter).second << endl;
        }
        os << endl;

        return os;
    }

    std::ostream& operator<<(std::ostream& os, const vector< vector< int> >& data) {
        cout << "cout << vector<vector<int> >\n";
        int row_size = data.size();
        int col_size = 0;
        if (row_size > 0) {
            col_size = data[0].size();
        }
        for (int i = 0; i < row_size; ++i) {
            for (int j = 0; j < col_size; ++j) {
                os << data[i][j] << " ";
            }
            os << endl;
        }
        return os;
    }
}

/* graphext namespace */
namespace graphext {
    void id_of_all_vertices(const Graph& g, std::set<std::string>& r) {
        BGL_FORALL_VERTICES_T(v, g, Graph) {
            r.insert(g[v].id);
        }
    }

    void print_v_index_std_map(const Graph& g, const VertexIndexStdMap& v_index_std_map) {
        std::list<std::string> outputs;

        VertexIndexStdMap::const_iterator iter;
        for (iter = v_index_std_map.begin(); iter != v_index_std_map.end(); ++iter) {
            outputs.push_back(std::to_string(iter->second));
            // outputs.push_back(std::to_string(&(iter->first)) + ": " + std::to_string(iter->second));
        }

        using namespace boost::spirit::karma;
        cout << "Vertex Index Std Map:\n";
        cout << format("[\n  " << (auto_ % "\n  ") << "\n]\n", outputs);
    }

    void print_v_index_map(const Graph& g, const VertexIndexMap& v_index_map) {
        cout << "Vertex Index Map:\n";

        std::list<std::string> outputs;
        BGL_FORALL_VERTICES_T(v, g, Graph) {
            int index = boost::get(v_index_map, v);
            std::string vertex_id = g[v].id;
            cout << v << endl;
            outputs.push_back(vertex_id + ": " + std::to_string(index));
        }

        using namespace boost::spirit::karma;
        cout << format("[\n  " << (auto_ % "\n  ") << "\n]\n", outputs);
    }
}