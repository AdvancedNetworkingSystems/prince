#include "graph_manager.h"

// CONSTRUCTOR
GraphManager::GraphManager(bool weighted_graph) : weighted_graph_(weighted_graph) {
    v_index_pmap_ = VertexIndexPMap(v_index_std_map_);
    e_index_pmap_ = EdgeIndexPMap(e_index_std_map_);
}

GraphManager::GraphManager(const GraphManager& other) {
#ifdef LOG
	BOOST_LOG_TRIVIAL(info) << "\n*******COPY CONSTRUCTOR******\n";
#endif
    g_ = other.g_;
    weighted_graph_ = other.weighted_graph_;
    ResetVerticesAndEdgesIndexMap();
}

GraphManager& GraphManager::operator=(const GraphManager& rhs) {
#ifdef LOG
	BOOST_LOG_TRIVIAL(info) << "\n*******ASSIGNMENT OPERATOR******\n";
#endif
    g_ = rhs.g_;
    weighted_graph_ = rhs.weighted_graph_;
    ResetVerticesAndEdgesIndexMap();

    return *this;
}

// GETTERS
const VertexIndexPMap& GraphManager::v_index_pmap() const {
    return v_index_pmap_;
}

const EdgeIndexPMap& GraphManager::e_index_pmap() const {
    return e_index_pmap_;
}
NameToIntMap const& GraphManager::v_id_index_map() const {
    return v_id_index_map_;
}

bool GraphManager::weighted_graph() const {
    return weighted_graph_;
}

// UPDATE GRAPHS
void GraphManager::AddEdge(VertexProperties vp1, VertexProperties vp2, EdgeProperties ep) {
#ifdef LOG
	BOOST_LOG_TRIVIAL(info)<< "add edge GM " << vp1.label << " - " << vp2.label << endl;
#endif

    string s1 = vp1.id;
    string s2 = vp2.id;

    if (s1 != s2) { // do not add self-loop into the graph
        Vertex v1;
        Vertex v2;

        try {
            v1 = get_vertex_from_id(s1);
        }
        catch (exception& e) {
            v1 = boost::add_vertex(vp1, g_);
            v_id_vertex_map_[s1] = v1;
            update_v_index_pmap(v1);
        }
        try {
            v2 = get_vertex_from_id(s2);
        }
        catch (exception& e) {
            v2 = boost::add_vertex(vp2, g_);
            v_id_vertex_map_[s2] = v2;
            update_v_index_pmap(v2);
        }

        Edge e;
        bool inserted;
        boost::tie(e, inserted) = boost::add_edge(v1, v2, ep, g_);
        update_e_index_pmap(e);
    }
}

void GraphManager::ResetVerticesAndEdgesIndexMap() {
    reset_v_index_pmap();
    reset_v_id_vertex_map();
    // The line below must be called after reset_v_index_pmap()
    reset_v_id_index_map();

    reset_e_index_pmap();
}

// HELPERS
bool GraphManager::vertex_existed(string s) {
    std::map<std::string, Vertex>::iterator it;
    it = v_id_vertex_map_.find(s);
    return (it != v_id_vertex_map_.end());
}

void GraphManager::get_degrees(vector<pair<string, int> > &map){
//	typedef typename boost::graph_traits<Graph>::vertex_iterator
//			vertex_iterator;
//	boost::graph_traits<Graph>::vertex_iterator vi, vi_end, next;
//	boost::tie(vi, vi_end) = boost::vertices(g_);
//	for(next= vi; vi != vi_end; vi=next){
//		pair<string, int> p;
//		p.first=g_[*vi].id;
//		p.second = degree(*vi, g_);
//		++next;
//
//	}
//
	BGL_FORALL_VERTICES(v, g_, Graph) {
		pair<string, int> p;
		int index = boost::get(v_index_pmap_, v);
		p.first = g_[v].label;
		p.second = degree(v, g_);
		map.push_back(p);
	}
	sort(map.begin(), map.end(), less_first<string, int>());
}

const Vertex& GraphManager::get_vertex_from_id(string s) {
    if (vertex_existed(s)) {
        return v_id_vertex_map_[s];
    }
    else {
        throw std::runtime_error("Vertex not found\n");
    }
}

int GraphManager::get_index_from_id(string s) {
    if (vertex_existed(s)) {
        return v_id_index_map_[s];
    }
    else {
        throw std::runtime_error("Vertex with id " + s + " is not found\n");
    }
}
//
//// OUTPUTTING THE RESULT
//void GraphManager::print() {
//    cout << "\nGraph Manager:\n";
//    outops::operator<<(cout, g_);
//
//    cout << "Is graph connected?\n";
//    bool connected = graphext::is_connected(g_, v_index_pmap_);
//    cout << "Connected = " << connected << endl;
//
//    cout << "Is this a weighted graph?\n";
//    cout << "is weighted = " << weighted_graph_ << endl;
//
//    cout << "v_id_index_map:\n";
//    outops::operator<< <int>(cout, v_id_index_map_);
//}
//
//void GraphManager::print_v_index_pmap() {
//    graphext::print_v_index_pmap(g_, v_index_pmap_);
//}
//
//void GraphManager::print_e_index_pmap() {
//    graphext::print_e_index_pmap(g_, e_index_pmap_);
//}
//
//std::ostream& operator<<(std::ostream& os, const GraphManager& gm) {
//    cout << "\nGraph Manager: " << endl;
//    outops::operator<<(cout, gm.g_);
//
//    cout << "v_id_index_map:\n";
//    outops::operator<< <int>(cout, gm.v_id_index_map());
//    return os;
//}

// Private Functions
void GraphManager::reset_v_id_vertex_map() {
    v_id_vertex_map_ = NameVertexMap();
    BGL_FORALL_VERTICES(v, g_, Graph) {
        string id = g_[v].id;
        v_id_vertex_map_[id] = v;
    }
}

void GraphManager::reset_v_id_index_map() {
    v_id_index_map_ = NameToIntMap();
    BGL_FORALL_VERTICES(v, g_, Graph) {
        int index = boost::get(v_index_pmap_, v);
        string name = g_[v].id;
        v_id_index_map_[name] = index;
    }
}

void GraphManager::reset_v_index_pmap() {
    v_index_std_map_ = VertexIndexStdMap();
    v_index_pmap_ = VertexIndexPMap(v_index_std_map_);
    int i = 0;
    BGL_FORALL_VERTICES(v, g_, Graph) {
        boost::put(v_index_pmap_, v, i);
        ++i;
    }
}

void GraphManager::reset_e_index_pmap() {
    e_index_std_map_ = EdgeIndexStdMap();
    e_index_pmap_ = EdgeIndexPMap(e_index_std_map_);
    int i = 0;
    BGL_FORALL_EDGES(e, g_, Graph) {
        boost::put(e_index_pmap_, e, i);
        ++i;
    }
}

void GraphManager::update_v_index_pmap(Vertex new_vertex) {
    // NOTE: this function might not perform correctly for vecS
    int index = boost::num_vertices(g_);
    v_index_std_map_[new_vertex] = index - 1;
}

void GraphManager::update_e_index_pmap(Edge new_edge) {
    int index = boost::num_edges(g_);
    e_index_std_map_[new_edge] = index - 1;
}
