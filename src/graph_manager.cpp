#include "graph_manager.h"

GraphManager::GraphManager() {
    v_index_map_ = VertexIndexMap(v_index_std_map_);
    e_index_map_ = EdgeIndexMap(e_index_std_map_);
}

GraphManager::GraphManager(const GraphManager& other) {
    cout << "\n\n*******COPY CONSTRUCTOR******\n\n";
    g_ = other.g_;
    reset_v_id_vertex_map();
    reset_v_index_map();
    reset_e_index_map();
}

GraphManager& GraphManager::operator=(GraphManager& rhs) {
    cout << "\n\n*******ASSIGNMENT OPERATOR******\n\n";
    g_ = rhs.g_;
    reset_v_id_vertex_map();
    reset_v_index_map();
    reset_e_index_map();

    return *this;
}

void GraphManager::AddEdge(VertexProperties vp1, VertexProperties vp2, EdgeProperties ep) {
    // cout << "add edge GM " << vp1.label << " - " << vp2.label << endl;

    string s1 = vp1.id;
    string s2 = vp2.id;
    Vertex v1;
    Vertex v2;

    try {
        v1 = get_vertex_from_id(s1);
    }
    catch (exception& e) {
        v1 = boost::add_vertex(vp1, g_);
        v_id_vertex_map_[s1] = v1;
        update_v_index_map(v1);
    }
    try {
        v2 = get_vertex_from_id(s2);
    }
    catch (exception& e) {
        v2 = boost::add_vertex(vp2, g_);
        v_id_vertex_map_[s2] = v2;
        update_v_index_map(v2);
    }

    Edge e;
    bool inserted;
    boost::tie(e, inserted) = boost::add_edge(v1, v2, ep, g_);
    update_e_index_map(e);
    // print_e_index_map();

    // Print the VertexIndexMap and EdgeIndexMap
    // print_v_index_map();
    // graphext::print_v_index_map(g_, v_index_map_);
}

bool GraphManager::vertex_existed(string s) {
    std::map<std::string, Vertex>::iterator it;
    it = v_id_vertex_map_.find(s);
    return (it != v_id_vertex_map_.end());
}

const Vertex& GraphManager::get_vertex_from_id(string s) {
    if (vertex_existed(s)) {
        return v_id_vertex_map_[s];
    }
    else {
        throw std::runtime_error("Vertex not found\n");
    }
}

void GraphManager::ResetVerticesAndEdgesIndexMap() {
    v_index_std_map_.erase(v_index_std_map_.begin(), v_index_std_map_.end());
    e_index_std_map_.erase(e_index_std_map_.begin(), e_index_std_map_.end());
    int i;

    // Reset VertexIndexMap
    i = 0;
    BGL_FORALL_VERTICES(v, g_, Graph) {
        boost::put(v_index_map_, v, i);
        ++i;
    }

    // Reset EdgeIndexMap
    i = 0;
    BGL_FORALL_EDGES(e, g_, Graph) {
        boost::put(e_index_map_, e, i);
        ++i;
    }
}

std::ostream& operator<<(std::ostream& os, const GraphManager& gm) {
    cout << "Graph Manager: " << endl;
    outops::operator<<(cout, gm.g_);
    return os;
}

void GraphManager::print_v_index_map() {
    graphext::print_v_index_map(g_, v_index_map_);
}

void GraphManager::print_e_index_map() {
    std::list<std::string> outputs;
    BGL_FORALL_EDGES_T(e, g_, Graph) {
        int index = boost::get(e_index_map_, e);
        std::string source_id = g_[boost::source(e, g_)].id;
        std::string target_id = g_[boost::target(e, g_)].id;
        outputs.push_back("edge (" + source_id + ", " + target_id + ")" + ": " + std::to_string(index));
    }

    using namespace boost::spirit::karma;
    cout << "Edge Index Map:\n";
    cout << format("[\n  " << (auto_ % "\n  ") << "\n]\n", outputs);
}

const VertexIndexMap& GraphManager::v_index_map() const {
    return v_index_map_;
}

const EdgeIndexMap& GraphManager::e_index_map() const {
    return e_index_map_;
}

void GraphManager::reset_v_id_vertex_map() {
    v_id_vertex_map_ = NameVertexMap();
    BGL_FORALL_VERTICES(v, g_, Graph) {
        string id = g_[v].id;
        v_id_vertex_map_[id] = v;
    }
}

void GraphManager::reset_v_index_map() {
    v_index_std_map_ = VertexIndexStdMap();
    v_index_map_ = VertexIndexMap(v_index_std_map_);
    int i = 0;
    BGL_FORALL_VERTICES(v, g_, Graph) {
        boost::put(v_index_map_, v, i);
        ++i;
    }
}

void GraphManager::reset_e_index_map() {
    e_index_std_map_ = EdgeIndexStdMap();
    e_index_map_ = EdgeIndexMap(e_index_std_map_);
    int i = 0;
    BGL_FORALL_EDGES(e, g_, Graph) {
        boost::put(e_index_map_, e, i);
        ++i;
    }
}

void GraphManager::update_v_index_map(Vertex new_vertex) {
    int index = boost::num_vertices(g_);
    v_index_std_map_[new_vertex] = index - 1;
}

void GraphManager::update_e_index_map(Edge new_edge) {
    int index = boost::num_edges(g_);
    e_index_std_map_[new_edge] = index - 1;
}