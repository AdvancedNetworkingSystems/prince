//
// Created by quynh on 1/9/16.
//

#include "sub_component.h"


SubComponent::SubComponent() {
    // do nothing
}

/* GETTERS & SETTERS & UPDATERS */
GraphManager const& SubComponent::gm() const {
    return gm_;
}

StringSet const& SubComponent::all_vertices_id() const {
    return all_vertices_id_;
}

StringSet const& SubComponent::art_points_id() const {
    return art_points_id_;
}

StringSet const& SubComponent::non_art_points_id() const {
    return non_art_points_id_;
}
NameToIntMap const& SubComponent::weight_map() const {
    // Returns the whole weight_map_, for all the vertices
    // This one is different from get_weight_map(name)
    return weight_map_;
}

NameToIntMap const& SubComponent::weight_reversed_map() const {
    return weight_reversed_map_;
}

vector< vector< int > > const& SubComponent::traffic_matrix() const {
    return traffic_matrix_;
}

/* CREATE SUB-COMPONENT */
void SubComponent::AddEdge(Router r1, Router r2, Link l) {
    gm_.AddEdge(r1, r2, l);
}

void SubComponent::FinalizeSubComponent(StringSet all_art_points_id) {
    // Create set of vertices id
    graphext::id_of_all_vertices(gm_.g_, all_vertices_id_);

    // Create set of this sub-component's articulation points id
    using namespace setops;
    art_points_id_ = all_vertices_id_ / all_art_points_id;

    // Create set of vertices id (such that those vertices are not articulation points)
    non_art_points_id_ = all_vertices_id_ - art_points_id_;

    // Create name -> index map
    int index = 0;
    for (string s : all_vertices_id_) {
        name_index_map_[s] = index;
        ++index;
    }
}


/* LINK WEIGHT CALCULATION */
void SubComponent::initialize_weight() {
    for (string s : art_points_id_) {
        update_weight_map(s, -1);
    }
}

int SubComponent::get_weight_map(string name) {
    // Return only the weight for the vertex with the given name.
    // Check out weight_map()
    if (stdhelper::exists(weight_map_, name)) {
        return weight_map_[name];
    }
    else {
        return 0;
    }
}

int SubComponent::get_weight_reversed_map(string name) {
    // Return only the 'weight reversed' for the vertex with the given name.
    // Check out weight_reversed_map(). Those 2 functions serve different purpose
    if (stdhelper::exists(weight_reversed_map_, name)) {
        return weight_reversed_map_[name];
    }
    else {
        return 0;
    }
}

void SubComponent::update_weight_map(string name, int value) {
    // cout << "update " << name << " = " << value << endl;
    weight_map_[name] = value;
}

/* TRAFFIC MATRIX */
void SubComponent::CalculateTrafficMatrix() {
    initialize_traffic_matrix();

    // When only one vertex is an articulation point
    for (string a : art_points_id_) {
        // cout << a << " ";
        for (string non_a : non_art_points_id_) {
            // cout << non_a << " ";
            int communication_intensity = get_weight_reversed_map(a) + 1;
            update_traffic_matrix(a, non_a, communication_intensity);
        }
        // cout << endl;
    }

    // When both vertices are articulation points
    int size = art_points_id_.size();
    for (string a1 : art_points_id_) {
        for (string a2 : art_points_id_) {
            if (a1.compare(a2) == 0)
                continue;

            int communication_intensity = (
                (get_weight_reversed_map(a1) + 1) *
                (get_weight_reversed_map(a2) + 1)
            );
            update_traffic_matrix(a1, a2, communication_intensity);
        }
    }
    // cout << "Mark 3\n";
}

void SubComponent::initialize_traffic_matrix() {
    // generate_empty_traffic_matrix, with 1 every where, and 0 in the main diagonal
    int size = num_of_vertices();
    traffic_matrix_ = vector< vector<int> >(size);
    for (int i = 0; i < size; ++i) {
        traffic_matrix_[i] = vector< int >(size, 1);
    }

    // Reset the main diagonal to 0
    for (int i = 0; i < size; ++i) {
        traffic_matrix_[i][i] = 0;
    }
}

int SubComponent::get_traffic_matrix(string name_1, string name_2) {
    int i1 = index_of_vertex_id(name_1);
    int i2 = index_of_vertex_id(name_2);
    // TODO: exception
    return traffic_matrix_[i1][i2];
}

void SubComponent::update_traffic_matrix(string name_1, string name_2, int value) {
    int i1 = index_of_vertex_id(name_1);
    int i2 = index_of_vertex_id(name_2);
    // cout << i1 << " " << i2 << " = " << value << endl;
    traffic_matrix_[i1][i2] = value;
    traffic_matrix_[i2][i1] = value; // because Traffic Matrix is symmetric
}

/* BETWEENNESS CENTRALITY */
void SubComponent::CalculateBetweennessCentrality() {
    initialize_betweenness_centrality();

    cout << "Mark 1" << endl;

    boost::brandes_betweenness_centrality(gm_.g_,
        boost::centrality_map(v_centrality_map_).vertex_index_map(
            gm_.v_index_map())
    );

    cout << "Mark 2" << endl;

    cout << "Vertex betweenness\n" << endl;
    for (int i = 0; i < num_of_vertices(); ++i) {
        cout << v_centrality_vec_.at(i) << endl;
    }
}

void SubComponent::initialize_betweenness_centrality() {
    v_centrality_vec_ = CentralityVec(num_of_vertices());
    v_centrality_map_ = CentralityMap(v_centrality_vec_.begin(), gm_.v_index_map());
}

/* HELPERS */
int SubComponent::num_of_vertices() {
    return boost::num_vertices(gm_.g_);
}

int SubComponent::index_of_vertex_id(string vertex_id) {
    // TODO: might throw exception here
    return name_index_map_[vertex_id];
}

bool SubComponent::vertex_exists(string name) {
    stdhelper::exists(art_points_id_, name);
}

string SubComponent::first_vertex_id_with_unknown_weight() {
    string vertex_id = "";
    for (auto &i : weight_map_) {
        if (i.second == -1) {
            vertex_id = i.first;
            break;
        }
    }
    return vertex_id;
}

/* HELPERS FOR OUTPUTTING RESULT */
void SubComponent::print_traffic_matrix() {

}

std::ostream& operator<<(std::ostream& os, const SubComponent& sc) {
    cout << "Sub-component:" << endl;
    cout << sc.gm_;

    cout << "\nArticulation points ID:\n";
    outops::operator<<(cout, sc.art_points_id());

    cout << "\nNormal Vertices ID:\n";
    outops::operator<<(cout, sc.all_vertices_id());

    cout << "\nLink Weight:\n";
    outops::operator<<(cout, sc.weight_map());
    // printhelper::for_map<string, int>(sc.weight_map());

    cout << "\nTraffic Matrix:\n";
    outops::operator<<(cout, sc.traffic_matrix());

    return os;
}
