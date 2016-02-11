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

std::map< std::pair<string, string>, int > const& SubComponent::traffic_matrix() const {
    return traffic_matrix_;
}

CentralityVec const& SubComponent::v_centrality_vec() const {
    return v_centrality_vec_;
}

/* CREATE SUB-COMPONENT */
void SubComponent::AddEdge(Router r1, Router r2, Link l) {
    gm_.AddEdge(r1, r2, l);
}

void SubComponent::FinalizeSubComponent(StringSet all_art_points_id) {
    // TODO: is there anyway to not have to call this function after the graph was created completely?
    gm_.ResetVerticesAndEdgesIndexMap();

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
        name_index_pmap_[s] = index;
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

void SubComponent::update_weight_reversed_map(string name, int value) {
    weight_reversed_map_[name] = value;
}

void SubComponent::calculate_weight_reversed(int V) {
    // V is the total number of vertices in the parent component
    for (string name : all_vertices_id_) {
        int value = get_weight_map(name);
        update_weight_reversed_map(name, V - 1 - value);
    }
}

/* TRAFFIC MATRIX */
void SubComponent::CalculateTrafficMatrix() {
    traffic_matrix_pmap_ = TrafficMatrixPMap(traffic_matrix_);

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
    // generate_empty_traffic_matrix, with 1 for different vertices, and 0 for the same vertices
    for (string id1 : all_vertices_id_) {
        for (string id2 : all_vertices_id_) {
            if (id1.compare(id2) == 0) {
                update_traffic_matrix(id1, id2, 0);
            }
            else {
                update_traffic_matrix(id1, id2, 1);
            }
        }
    }
}

int SubComponent::get_traffic_matrix(string name_1, string name_2) {
    std::pair<string, string> p;
    if (name_1.compare(name_2) <= 0) {
        p = std::pair<string, string>(name_1, name_2);
    }
    else {
        p = std::pair<string, string>(name_2, name_1);
    }
    return traffic_matrix_[p];
}

void SubComponent::update_traffic_matrix(string name_1, string name_2, int value) {
    std::pair<string, string> p;
    if (name_1.compare(name_2) <= 0) {
        p = std::pair<string, string>(name_1, name_2);
    }
    else {
        p = std::pair<string, string>(name_2, name_1);
    }
    traffic_matrix_[p] = value;
}

/* BETWEENNESS CENTRALITY */
void SubComponent::CalculateBetweennessCentralityHeuristic() {
    initialize_betweenness_centrality();

    boost::brandes_betweenness_centrality_heuristic(gm_.g_,
        traffic_matrix_pmap_,
        boost::centrality_map(
            v_centrality_pmap_).vertex_index_map(
            gm_.v_index_pmap())
    );
}

void SubComponent::initialize_betweenness_centrality() {
    v_centrality_vec_ = CentralityVec(num_of_vertices());
    v_centrality_pmap_ = CentralityPMap(v_centrality_vec_.begin(), gm_.v_index_pmap());
}

double SubComponent::get_betweenness_centrality(string name) {
    // There are 2 ways to retrieve the BC score
    // 1st way - through v_centrality_vec_
    int index = gm_.get_index_from_id(name);
    return v_centrality_vec_.at(index);
}

double SubComponent::get_betweenness_centrality(Vertex v) {
    // 2nd way - uncomment to use v_centrality_pmap
    return boost::get(v_centrality_pmap_, v);
}

/* HELPERS */
int SubComponent::num_of_vertices() {
    return boost::num_vertices(gm_.g_);
}

int SubComponent::index_of_vertex_id(string vertex_id) {
    // TODO: might throw exception here
    return name_index_pmap_[vertex_id];
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
void SubComponent::print() {
    cout << "Sub-component:" << endl;
    gm_.print();

    cout << "\nArticulation points ID:\n";
    outops::operator<<(cout, art_points_id());

    cout << "\nNormal Vertices ID:\n";
    outops::operator<<(cout, all_vertices_id());

    cout << "\nLink Weight:\n";
    outops::operator<< <int> (cout, weight_map());
    // printhelper::for_map<string, int>(sc.weight_map());

    cout << "\nTraffic Matrix:\n";
    print_traffic_matrix();

    cout << "\nBetweenness Centrality:\n";
    outops::operator<< <double>(cout, v_centrality_vec());
}

void SubComponent::print_traffic_matrix() {
    typedef std::map<std::pair<string, string>, int>::const_iterator Iter;

    for (auto elem : traffic_matrix_) {
        cout << elem.first.first << " - " << elem.first.second << ": " << elem.second << endl;
    }
    // for (Iter iter = traffic_matrix_.begin(); iter != traffic_matrix_.end(); ++iter) {
    //     cout << *(iter->first) << " - " << endl;
    // }
}

std::ostream& operator<<(std::ostream& os, const SubComponent& sc) {
    cout << "Sub-component:" << endl;
    cout << sc.gm_;

    cout << "\nArticulation points ID:\n";
    outops::operator<<(cout, sc.art_points_id());

    cout << "\nNormal Vertices ID:\n";
    outops::operator<<(cout, sc.all_vertices_id());

    cout << "\nLink Weight:\n";
    outops::operator<< <int> (cout, sc.weight_map());
    // printhelper::for_map<string, int>(sc.weight_map());

    cout << "\nTraffic Matrix:\n";
    // I didn't write the << for traffic_matrix
    // outops::operator<<(cout, sc.traffic_matrix());

    cout << "\nBetweenness Centrality:\n";
    outops::operator<< <double>(cout, sc.v_centrality_vec());

    return os;
}
