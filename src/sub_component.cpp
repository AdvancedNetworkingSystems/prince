//
// Created by quynh on 1/9/16.
//

#include "sub_component.h"


SubComponent::SubComponent() {
    // do nothing
}

SubComponent::SubComponent(StringSet art_points, Graph sub_graph) : art_points_(art_points), sub_graph_(sub_graph) {
// SubComponent::SubComponent(VertexVec aart_points, Graph asubGraph) {
    // art_points = aart_points;
    // subGraph = asubGraph;
    // Deep copy for art_points
    // cout << "ABC " << endl;
    // for (VertexVecIter vi = aart_points.begin(); vi != aart_points.end(); ++vi) {
    //     Vertex v = Vertex(*vi);
    //     this->art_points.insert(this->art_points.end(), v);
    //     cout << "   asubGraph " << asubGraph[*vi].name << endl;
    //     cout << "    subGraph " << subGraph[*vi].name << endl;
    // }


    // init();
}

StringSet SubComponent::art_points() const {
    return art_points_;
}

void SubComponent::set_art_points(StringSet& art_points) {
    art_points_ = art_points;
}

int SubComponent::num_vertices() {
    return boost::num_vertices(sub_graph_);
}

void SubComponent::AddEdge(Router r1, Router r2, Link l) {
    cout << "add edge " << r1.label << " - " << r2.label << endl;

    string s1 = r1.id;
    string s2 = r2.id;
    Vertex v1;
    Vertex v2;

    try {
        v1 = get_vertex_from_id(s1);
    }
    catch (exception& e) {
        v1 = boost::add_vertex(r1, sub_graph_);
        name_vertex_map_[s1] = v1;
    }
    try {
        v2 = get_vertex_from_id(s2);
    }
    catch (exception& e) {
        v2 = boost::add_vertex(r2, sub_graph_);
        name_vertex_map_[s2] = v2;
    }
    boost::add_edge(v1, v2, l, sub_graph_);
}

bool SubComponent::vertex_existed(string s) {
    std::map<std::string, Vertex>::iterator it;
    it = name_vertex_map_.find(s);
    return (it != name_vertex_map_.end());
}

const Vertex& SubComponent::get_vertex_from_id(string s) {
    if (vertex_existed(s)) {
        return name_vertex_map_[s];
    }
    else {
        throw std::runtime_error("Vertex not found\n");
    }
}

std::ostream& operator<<(std::ostream& os, const SubComponent& sc) {
    cout << "Sub-component" << endl;
    outops::operator<<(cout, sc.sub_graph());
    outops::operator<<(cout, sc.art_points());
    return os;
}

Graph const& SubComponent::sub_graph() const {
    return sub_graph_;
}

