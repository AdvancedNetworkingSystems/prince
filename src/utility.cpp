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

    bool is_connected(const Graph& g, const VertexIndexPMap& v_index_pmap) {
        Vertex v = *(boost::vertices(g).first);

        // This one shows a nice way to incoperate the index, discover_time into VertexProperties
        // http://www.boost.org/doc/libs/1_58_0/libs/graph/example/bfs-example2.cpp
        typedef boost::graph_traits < Graph >::vertices_size_type Size;
        typedef boost::iterator_property_map < std::vector< Size >::iterator,
            VertexIndexPMap >  dtime_pm_t;

        std::vector < Size > dtime(num_vertices(g));
        dtime_pm_t dtime_pm(dtime.begin(), v_index_pmap);

        Size time = 0;
        bfs_time_visitor < dtime_pm_t, Size >vis(dtime_pm, time);
        boost::breadth_first_search(g, v, boost::vertex_index_map(v_index_pmap).visitor(vis));

        if (dtime.size() == boost::num_vertices(g)) {
            return true; // BFS discovers all vertices in the graph, so the graph is connected
        }
        else {
            return false;
        }
    }

    void print_edge(const Graph& g, const Edge& e) {
        string s = g[boost::source(e, g)].id;
        string t = g[boost::target(e, g)].id;

        printf("edge (%s, %s)", s.c_str(), t.c_str());
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

    void write_betweenness_centrality(Graph const& g, std::vector<double> v_centrality_vec, string file_path) {
        cout << "XXX Writing to File";
        // string filepath = "../output/boost_" + fileSuffix + ".csv";
        ofstream out_file(file_path.c_str());

        Viter vi, ve;
        size_t i = 0;
        if (out_file.is_open()) {
            for (boost::tie(vi, ve) = boost::vertices(g); vi != ve; ++vi) {
                out_file << g[*vi].id << ", " << v_centrality_vec.at(i) << endl;
                ++i;
            }
        }
        out_file.close();

        cout << "Done Writing BC score to file " << file_path << endl;
    }
}

// GENERAL HELPERS
namespace helper {
    string get_file_name(const string& s) {
       char sep = '/';

    #ifdef _WIN32
       sep = '\\';
    #endif

       size_t i = s.rfind(sep, s.length());
       if (i != string::npos) {
          return(s.substr(i+1, s.length() - i));
       }

       return("");
    }
}
