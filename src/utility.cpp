//
// Created by quynh on 12/15/15.
//

#include "utility.h"
using namespace boost;

/* outops namespace */
namespace outops {
    std::ostream& operator<<(std::ostream& os, const Graph& g) {
        os <<   "Graph has: \n"
                "---------- " << boost::num_vertices(g) << " vertices\n"
                "---------- " << boost::num_edges(g) << " edges\n";

        std::vector<std::string> verticesVec;
        BGL_FORALL_VERTICES(v, g, Graph) verticesVec.push_back(g[v].id);

        std::vector<std::string> edgesVec;
        std::vector<double> costsVec;
        BGL_FORALL_EDGES(e, g, Graph) {
            std::string s = "(" + g[e.m_source].id + ", " + g[e.m_target].id + ") - " + std::to_string(g[e].cost);
            edgesVec.push_back(s);
            costsVec.push_back(g[e].cost);
        }

        using namespace boost::spirit::karma;
        os <<   "Vertices:\n"
                "  ";
        os << format("(" << auto_ % ", " << ") ", verticesVec);
        os << "\n";

        os <<   "Edges:\n";
        os << format("  " << (auto_ % "\n  ") << eol, edgesVec);
        os << "\n";

        return os;
    }

    std::ostream& operator<<(std::ostream& os, std::pair<const Graph&, const VertexIndexPMap&> p) {
        // ERROR: wrong output.
        // I think it's because of copy constructor.
        // Check out shell_output/w14

        // Calling example:
        // outops::operator<<(cout, std::pair<const Graph&, const VertexIndexPMap&>(g_, v_index_pmap_));
        Graph g = p.first;
        VertexIndexPMap v_index_map = p.second;

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

    void print_v_index_pmap(const Graph& g, const VertexIndexPMap& v_index_pmap) {
        std::list<std::string> outputs;
        BGL_FORALL_VERTICES_T(v, g, Graph) {
            int index = boost::get(v_index_pmap, v);
            std::string vertex_id = g[v].id;
            // Uncomment to print the address of vertex v
            // cout << v << endl;
            outputs.push_back(vertex_id + ": " + std::to_string(index));
        }

        using namespace boost::spirit::karma;
        cout << "Vertex Index Map:\n";
        cout << format("[\n  " << (auto_ % "\n  ") << "\n]\n", outputs);
    }

    void print_e_index_pmap(const Graph& g, const EdgeIndexPMap& e_index_pmap) {
        std::list<std::string> outputs;
        BGL_FORALL_EDGES_T(e, g, Graph) {
            int index = boost::get(e_index_pmap, e);
            std::string source_id = g[boost::source(e, g)].id;
            std::string target_id = g[boost::target(e, g)].id;
            outputs.push_back("edge (" + source_id + ", " + target_id + ")" + ": " + std::to_string(index));
        }

        using namespace boost::spirit::karma;
        cout << "Edge Index Map:\n";
        cout << format("[\n  " << (auto_ % "\n  ") << "\n]\n", outputs);
    }
}