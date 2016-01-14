//
// Created by quynh on 1/9/16.
//

#include "bi_connected_components.h"
using namespace std;

BiConnectedComponents::BiConnectedComponents(const Graph &g) : g(g) {
    init();
    findBiConnectedComponents();
}


void BiConnectedComponents::init() {
    size_t i = 0; // for indexing in the loop

    // Populate the e_index_map, by changing the value of the original e_index_std_map
    e_index_map = EdgeIndexMap(e_index_std_map);
    i = 0;
    BGL_FORALL_EDGES(edge, g, Graph) {
        e_index_std_map.insert(pair<Edge, size_t>(edge, i));
        ++i;
    }

    this->component_vec = ComponentVec(boost::num_edges(g), 0);
    this->component_map = ComponentMap(this->component_vec.begin(), e_index_map);

    // Populate the VertexIndexMap - by using the put() from Property Map
    v_index_map = VertexIndexMap(v_index_std_map);
    Viter vi, ve; // vertex_iterator, vertex_iterator_end
    i = 0;
    for (boost::tie(vi, ve) = boost::vertices(g); vi != ve; ++vi) {
        boost::put(this->v_index_map, *vi, i);
        ++i;
    }
}

void BiConnectedComponents::findBiConnectedComponents() {
    cout << "Running findBiConnectedComponents" << endl;
    size_t i;


    boost::biconnected_components(g, component_map,
                                  back_inserter(art_points),
                                  boost::vertex_index_map(this->v_index_map));

    cout << "Articulation points: \n";
    set<string> art_point_ids;
    graphext::id_of_vertices(g, art_points, art_point_ids);
    outops::operator<<(cout, art_point_ids);

    // Process the result from boost::biconnected_components
    BCCs = Component_t(num_bcc());
    verticesInBCC();
    // testVerticesInBCC();
    processArtPoints();
    // testProcessArtPoints();
    // _test_set_difference();
    createSubComponents();
}

void BiConnectedComponents::createSubComponents() {
    // Generating articulation points for each sub-component
    for (int i = 0; i < num_bcc(); ++i) {
        StringSet art_point_ids;
        VertexVec art_points_vec;
        art_points_vec = get_art_points_for_component(i);
        graphext::id_of_vertices(g, art_points_vec, art_point_ids);

        outops::operator<<(cout, art_point_ids);
        BCCs[i].set_art_points(art_point_ids);
    }

    // Generating subgraph for each sub-component
    BGL_FORALL_EDGES(edge, g, Graph) {
        int comp_index = boost::get(component_map, edge);
        Router r1 = g[boost::source(edge, g)];
        Router r2 = g[boost::target(edge, g)];
        Link l = g[edge];
        BCCs[comp_index].AddEdge(r1, r2, l);
    }

    for (int i = 0; i < num_bcc(); ++i) {
        cout << BCCs[i] << endl;
    }
}

void BiConnectedComponents::print() {
    // Test the biconnected components
    Eiter ei, ei_end;
    size_t i = 0;
    for (boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei) {
        string source = g[(*ei).m_source].id;
        string target = g[(*ei).m_target].id;
        edges_size_type comp = this->component_vec.at(i);
        cout << source << " " << target << " " << comp << endl;
        ++i;
    }
}

int BiConnectedComponents::num_bcc() {
    if (num_of_bcc == -1) { // calculate it
        // +1 to counteract for the zero-indexing
        num_of_bcc = *std::max_element(component_vec.begin(), component_vec.end()) + 1;
    }

    return num_of_bcc;
}

void BiConnectedComponents::verticesInBCC() {
    size_t size = num_bcc();

    vector<set<Vertex> > set_vertices(size);

    Edge edge;
    Vertex source;
    Vertex target;

    int comp_id;
    BGL_FORALL_EDGES(edge, g, Graph) {
        comp_id = boost::get(component_map, edge);
        source = edge.m_source;
        target = edge.m_target;
        set_vertices[comp_id].insert(source);
        set_vertices[comp_id].insert(target);
    }

    bcc_vertices = vector<VertexVec>(size);
    size_t i = 0;
    for (i = 0; i < size; ++i) {
        bcc_vertices[i] = vector<Vertex>(set_vertices[i].begin(), set_vertices[i].end());
    }
}

void BiConnectedComponents::testVerticesInBCC() {
    size_t i = 0;
    for (i; i < num_of_bcc; ++i) {
        vector<Vertex> abc = bcc_vertices[i];
        cout << "vertices in BCC " << i << endl;
        for (int j = 0; j < abc.size(); ++j) {

            cout << g[bcc_vertices[i][j]].id << endl;
        }
    }
}

bool BiConnectedComponents::hasVertex(Vertex v, int comp_index) {
    vector<Vertex> vv = bcc_vertices[comp_index];
    vector<Vertex>::iterator it;

    it = std::find(vv.begin(), vv.end(), v);
    if (it == vv.end()) {
        return false;
    }
    else {
        return true;
    }
}

void BiConnectedComponents::processArtPoints() {
    // For each articulation points, quickly identify (through the std::map) all the component containing those points.
    art_point_component_map = ArtPointComponentMap(art_point_component_std_map);

    std::vector<Vertex>::iterator vi = art_points.begin();
    std::vector<int>::iterator ii;

    for (vi; vi < art_points.end(); ++vi) {
        Vertex articulation_point = *vi;

        size_t i = 0;
        std::vector<int> components;
        for (i = 0; i < num_of_bcc; ++i) {
            ii = components.end();
            if (hasVertex(articulation_point, i)) {
                components.insert(ii, i);
            }
        }
        boost::put(art_point_component_map, articulation_point, components);
    }

    // For each component, quickly identify (through the std::map) all the articulation points belonging to it.
    component_art_point_map = ComponentArtPointMap(component_art_point_std_map);

    BccIter_t bi = bcc_vertices.begin();
    BccIter_t bi_end = bcc_vertices.end();

    VertexVecIter vvi = art_points.begin();
    VertexVecIter vvi_end = art_points.end();

    int i = 0;
    for (bi; bi != bi_end; ++bi) {
        // intersection
        VertexVec component = *bi;
        VertexVec intersection_result;
        std::set_intersection(component.begin(), component.end(), vvi, vvi_end, back_inserter(intersection_result));

        boost::put(component_art_point_map, i, intersection_result);
        ++i;

    }
}


void BiConnectedComponents::testProcessArtPoints() {
    std::vector<Vertex>::iterator vi = art_points.begin();
    std::vector<int>::iterator ii;

    for (vi; vi < art_points.end(); ++vi) {
        Vertex articulation_point = *vi;

        std::vector<int> components = boost::get(art_point_component_map, articulation_point);

        cout << "Components belonging to articulation point " << g[articulation_point].label << endl;

        for (ii = components.begin(); ii < components.end(); ++ii) {
            cout << *ii << endl;
        }
    }

    VertexVecIter vvi;
    int i = 0;
    for (i; i < num_bcc(); ++i) {
        VertexVec vv = boost::get(component_art_point_map, i);

        cout << "Articulation points from the component " << i << endl;

        for (vvi = vv.begin(); vvi != vv.end(); ++vvi) {
            cout << *vvi << endl;
        }
    }
}

VertexVec BiConnectedComponents::get_art_points_for_component(int comp_index) {
    return boost::get(component_art_point_map, comp_index);
}

vector<int> BiConnectedComponents::get_components_for_art_point(Vertex vertex) {
    return boost::get(art_point_component_map, vertex);
}

int BiConnectedComponents::num_of_vertices_in_component(int comp_index) {
    return bcc_vertices[comp_index].size();
}

VertexVec BiConnectedComponents::get_normal_vertices_for_component(int comp_index) {
    VertexVec normal_points;
    std::set_difference(bcc_vertices[comp_index].begin(),
                        bcc_vertices[comp_index].end(),
                        art_points.begin(),
                        art_points.end(),
                        back_inserter(normal_points)

    );
    return normal_points;
}

// void BiConnectedComponents::compute_weight() {
//     _initialize_weight();
//     _initialize_queue();

//     while (!Q.empty()) {
//         QueueElem elem = Q.front();
//         Q.pop();
//         int comp_index = elem.component_index;
//         Vertex current_art_point = elem.art_point;

//         if (elem.type == "component_vertex_pair") {
//             int size = BCCs[comp_index].num_vertices() - 1;
//             VertexVec art_points = BCCs[comp_index].art_points();

//             // TODO: I assume here that vvi != art_points.end(), aka. the element is always found.
//             VertexVecIter vvi;
//             vvi = std::find(art_points.begin(), art_points.end(), current_art_point);
//             try {
//                 art_points.erase(vvi);
//             }
//             catch (exception& e) {
//                 cout << "Standard exception: " << e.what() << endl;
//             }

//             for (vvi = art_points.begin(); vvi != art_points.end(); ++vvi) {
//                 cout << "    " << g[*vvi].label << endl;
//                 int weight = BCCs[comp_index].getWeight(*vvi);
//                 if (weight != -1) {
//                     size += boost::num_vertices(g) - weight - 1;
//                 }
//             }

//             int link_weight = size;
// //            _verify_weight(comp_index, current_art_point, link_weight);
//             BCCs[comp_index].setWeight(current_art_point, link_weight);

//             _find_unknown_weight_wrt_art_point(current_art_point);
//         }

//         if (elem.type == "vertex_component_pair") {
//             vector<int> comp_indices = get_components_for_art_point(current_art_point);;

//             vector<int>::iterator vi;
//             vi = std::find(comp_indices.begin(), comp_indices.end(), comp_index);
//             try {
//                 comp_indices.erase(vi);
//             }
//             catch (exception& e) {
//                 cout << "Standard exception: " << e.what() << endl;
//             }

//             int size = 0;
//             for (vi = comp_indices.begin(); vi != comp_indices.end(); ++vi) {
//                 int weight = BCCs[*vi].getWeight(current_art_point);
//                 if (weight != -1) {
//                     size += weight;
//                 }
//             }

//             int link_weight = boost::num_vertices(g) - 1 - size;
// //            _verify_weight(comp_index, current_art_point, link_weight);
//             BCCs[comp_index].setWeight(current_art_point, link_weight);
//             _find_unknown_weight_wrt_component(comp_index);

//         }
//     }
// }

// void BiConnectedComponents::_initialize_weight() {
//     ComponentIter_t ci;
//     for (ComponentIter_t ci = BCCs.begin(); ci != BCCs.end(); ++ci) {
//         (*ci)._initialize_weight();
//     }
// }

// void BiConnectedComponents::_initialize_queue() {
//     VertexVecIter vvi;

//     int i = 0;
//     VertexVec current_art_points;
//     for (i; i < num_bcc(); ++i) {
//         current_art_points = BCCs[i].art_points();
//         if (curr ent_art_points.size() == 1) {
//             // creating element for queue Q
//             Vertex art_point = current_art_points[0];
//             string type = "component_vertex_pair";
//             QueueElem qe = {
//                     .component_index = i,
//                     .art_point = art_point,
//                     .type = type,
//             };

//             Q.push(qe);
//         }
//     }
// }

// void BiConnectedComponents::_find_unknown_weight_wrt_art_point(Vertex art_point) {
//     int num_of_uncomputed_weight = 0;
//     vector<int> uncomputed_weight_component_indices;

//     size_t i;
//     for (i = 0; i < num_bcc(); ++i) {
//         if (hasVertex(art_point, i)) {
//             // Check if value -1 appear exactly 1 time
//             if (BCCs[i].getWeight(art_point) == -1) {
//                 num_of_uncomputed_weight += 1;
//                 uncomputed_weight_component_indices.insert(uncomputed_weight_component_indices.end(), i);
//             }
//         }

//         if (num_of_uncomputed_weight > 1) {
//             break;
//         }
//     }

//     if (num_of_uncomputed_weight == 1) {
//         QueueElem elem = {
//                 component_index: uncomputed_weight_component_indices[0],
//                 art_point: art_point,
//                 type: "vertex_component_pair",
//         };
//         Q.push(elem);
//     }
// }

// void BiConnectedComponents::_find_unknown_weight_wrt_component(int comp_index) {
//     VertexVec unknown_weight_vertices;
//     BCCs[comp_index]._find_vertices_with_unknown_weight(unknown_weight_vertices);

//     if (unknown_weight_vertices.size() == 1) {
//         QueueElem elem = {
//                 .component_index = comp_index,
//                 .art_point = unknown_weight_vertices[0],
//                 .type = "component_vertex_pair",
//         };

//         Q.push(elem);
//     }
// }

// void BiConnectedComponents::print_weight() {
//     for (int i = 0; i < num_bcc(); ++i) {
//         BCCs[i].printWeight();
//     }
// }

// void BiConnectedComponents::findBetweennessCentrality() {
//     for (int i = 0; i < num_bcc(); ++i) {
//         // BCCs[i]._computeTrafficMatrix();
//         cout << "###########" << endl;
//         _print_art_points();
//         cout << "###########" << endl;
//         BCCs[i].findBetweennessCentrality();
//     }
// }

// void BiConnectedComponents::printBetweennessCentrality() {
//     for (int i = 0; i < num_bcc(); ++i) {
//         BCCs[i].printBetweennessCentrality();
//     }
// }

// void BiConnectedComponents::_print_art_points() {
//     for (int i = 0; i < art_points.size(); ++i) {
//         cout << &art_points[i] << endl;
//     }
// }

// void BiConnectedComponents::_test_set_difference() {
//     VertexVec component = bcc_vertices[0];
//     VertexVec intersection_result;

//     cout << "******** Test set_difference ********" << endl;
//     cout << "  Component" << endl;
//     for (VertexVecIter vvi = component.begin(); vvi != component.end(); ++vvi) {
//         cout << "\t" << *vvi << endl;
//     }
//     cout << "  Articulation points" << endl;
//     cout << "  " << &art_points[0] << endl;
//     cout << "  " << &art_points[1] << endl;

//     for (VertexVecIter vvi = art_points.begin(); vvi != art_points.end(); ++vvi) {
//         cout << "\t" << *vvi << endl;
//     }

//     VertexVec copy_art_points = VertexVec(art_points);
//     cout << "  Copy Articulation points" << endl;
//     cout << "  " << &copy_art_points << endl;
//     // cout << "  " << &(*copy_art_points) << endl;
//     cout << "  " << &(copy_art_points[0]) << " " << copy_art_points[0] << endl;
//     cout << "    " << &(*(&copy_art_points[0])) << " " << *(&copy_art_points[0]) << endl;
//     cout << "  " << &copy_art_points[1] << " " << copy_art_points[1] << endl;
//     for (VertexVecIter vvi = copy_art_points.begin(); vvi != copy_art_points.end(); ++vvi) {
//         cout << "\t" << *vvi << endl;
//     }

//     std::set_intersection(component.begin(), component.end(), art_points.begin(), art_points.end(), back_inserter(intersection_result));

//     cout << "  Intersection result" << endl;
//     for (VertexVecIter vvi = intersection_result.begin(); vvi != intersection_result.end(); ++vvi) {
//         cout << "\t" << *vvi << endl;
//     }

//     VertexVec difference_result;
//     std::set_intersection(component.begin(), component.end(), art_points.begin(), art_points.end(), back_inserter(difference_result));

//     cout << "  Difference result" << endl;
//     for (VertexVecIter vvi = difference_result.begin(); vvi != difference_result.end(); ++vvi) {
//         cout << "\t" << *vvi << endl;
//     }
// }