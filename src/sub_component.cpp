//
// Created by quynh on 1/9/16.
//

#include "sub_component.h"


SubComponent::SubComponent() {
}

SubComponent::SubComponent(VertexVec aart_points, Graph asubGraph) : art_points(aart_points), subGraph(asubGraph) {
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

    // Test if deep copy is performed for aart_points
    cout << "Test if deep copy is performed for art_points" << endl;
    cout << "&art_points " <<  &aart_points << endl;
    cout << "&this->art_points " << &this->art_points << endl;
    cout << "art_points[0] " <<  aart_points[0] << endl;
    cout << "this->art_points[0] " << this->art_points[0] << endl;

     // create Vertex -> Index Mapping
    v_index_map = VertexIndexMap(v_index_std_map);
    int i = 0;
    BGL_FORALL_VERTICES(vertex, this->subGraph, Graph) {
            boost::put(v_index_map, vertex, i);
            ++i;
    }

    // Deep copy for subGraph
    // TODO: optimize - is there anyway that we can avoid using deep copy

    // This is not working
    // copy_graph(asubGraph, this->subGraph, vertex_index_map(v_index_map));

    // this->subGraph = Graph(asubGraph);

    // Vertices in this->subGraph
    cout << "Vertices in this->subGraph | constructor" << endl;
    BGL_FORALL_VERTICES(vertex, this->subGraph, Graph) {
        cout << vertex << " " << this->subGraph[vertex].name << endl;
    }

    cout << "Vertices in asubGraph | constructor" << endl;
    BGL_FORALL_VERTICES(vertex, asubGraph, Graph) {
        cout << vertex << " " << asubGraph[vertex].name << endl;
    }
    init();
}

void SubComponent::init() {
    // create list of normal vertices
    Viter vi, vi_end;
    boost::tie(vi, vi_end) = boost::vertices(subGraph);
    std::set_difference(vi, vi_end,
                        art_points.begin(), art_points.end(),
                        back_inserter(normal_vertices)
    );


    cout << "Vertices in this->subGraph | init()" << endl;
    BGL_FORALL_VERTICES(vertex, this->subGraph, Graph) {
        cout << vertex << " " << this->subGraph[vertex].name << endl;
    }

    cout << "Test normal_vertices | init()" << endl;
    for (vector<Vertex>::iterator vi = normal_vertices.begin(); vi != normal_vertices.end(); ++vi) {
        cout << *vi << " ";
        cout << subGraph[*vi].name << endl;
    }

    /* STEP 1
    ** Compute Link Weight
    ** ==> Link Weight is computed by the main Bi Connected Components.
    ** That main BCC will set the Link Weight value for each sub-component.
    */

    /* STEP 2
    ** Compute Traffic Matrix
    */
//    _initializeTrafficMatrix();
//    _computeTrafficMatrix();

    /* STEP 3
    ** Compute the Betweenness Centrality
    ** The calculation is executed by the main BCCs calling each sub-component
    */
    // Initialize variables for calculating BC
    _initializeBetweennessCentrality();
    findBetweennessCentrality();
}

VertexVec SubComponent::get_art_points() {
    return art_points;
}

int SubComponent::num_vertices() {
    return boost::num_vertices(subGraph);
}

void SubComponent::_initialize_weight() {
    for (VertexVecIter vvi = art_points.begin(); vvi != art_points.end(); ++vvi) {
        setWeight(*vvi, -1);
    }
}

void SubComponent::setWeight(Vertex art_point, int value) {
    weightMap[art_point] = value;
}

int SubComponent::getWeight(Vertex art_point) {
    VertexMapIter vmi;

    vmi = weightMap.find(art_point);

    if (vmi != weightMap.end()) {
        return weightMap.at(art_point);
    }
    else {
        return 0;
    }
}

int SubComponent::getReversedWeight(Vertex art_point) {
    VertexMapIter vmi;

    vmi = weightMap.find(art_point);

    if (vmi != weightMap.end()) {
        return (boost::num_vertices(subGraph) - weightMap.at(art_point) - 1);
    }
    else {
        return 0;
    }
}

void SubComponent::printWeight() {
    for (VertexMapIter vmi = weightMap.begin(); vmi != weightMap.end(); ++vmi) {
        cout << subGraph[(*vmi).first].name << " = " << (*vmi).second << endl;
    }
}

void SubComponent::_find_vertices_with_unknown_weight(VertexVec& unknown_weight_vertices) {
    VertexMapIter wi;
    Vertex vertex;

    int current_weight;
    for (wi = weightMap.begin(); wi != weightMap.end(); ++wi) {
        vertex = (*wi).first;
        current_weight = (*wi).second;

        if (current_weight == -1) {
            unknown_weight_vertices.insert(unknown_weight_vertices.end(), vertex);
        }
    }
}

void SubComponent::_initializeTrafficMatrix() {
    // generate_empty_traffic_matrix, with 1 every where, and 0 in the main diagonal
    int size = boost::num_vertices(subGraph);
    trafficMatrix = vector<vector<int> >(size);
    for (int j = 0; j < size; ++j) {
        trafficMatrix[j] = vector<int>(size, 1);
    }

    // Reset the main diagonal to 0
    for (int j = 0; j < size; ++j) {
        trafficMatrix[j][j] = 0;
    }
}

void SubComponent::_setTrafficMatrix(Vertex v_1, Vertex v_2, int value) {
    int i_1 = _indexOfVertex(v_1);
    int i_2 = _indexOfVertex(v_2);
    trafficMatrix[i_1][i_2] = value;
    trafficMatrix[i_2][i_1] = value; // because Traffic Matrix is symmetric
}

int SubComponent::_indexOfVertex(Vertex v) {
    try {
        return boost::get(v_index_map, v);
    }
    catch (exception& e) {
        // TODO handle exception here
        cout << "ERROR _indexOfVertex() " << e.what() << endl;
        return -1;
    }
}

void SubComponent::_computeTrafficMatrix() {
    // Update the value when one vertex is a cut-point, another vertex is not a cut-point
    for (int j = 0; j < art_points.size(); ++j) {
        for (int k = 0; k < normal_vertices.size(); ++k) {
            int communication_intensity = getReversedWeight(art_points[j]) + 1;
            _setTrafficMatrix(art_points[j], normal_vertices[k], communication_intensity);
        }
    }

    // Update the value when both vertices are cut-points
    int size = art_points.size();
    if (size > 1) {
        for (int j = 0; j < size - 1; ++j) {
            for (int k = 1; k < size; ++k) {
                if (j == k) {
                    continue;
                }

                int communication_intensity = (
                        (getReversedWeight(art_points[j]) + 1) *
                        (getReversedWeight(art_points[k]) + 1)
                );

                _setTrafficMatrix(art_points[j], art_points[k], communication_intensity);
            }
        }
    }
}

int SubComponent::getTrafficMatrix(Vertex v_1, Vertex v_2) {
    int i_1 = _indexOfVertex(v_1);
    int i_2 = _indexOfVertex(v_2);
    try {
        return trafficMatrix[i_1][i_2];
    }
    catch (exception& e) {
        cout << "ERROR: getTrafficMatrix: " << e.what() << endl;
    }

}

void SubComponent::_initializeBetweennessCentrality() {
    v_centrality_vec = CentralityVec(boost::num_vertices(subGraph), 0);
    v_centrality_map = CentralityMap(v_centrality_vec.begin(), v_index_map);
    cout << "Test v_index_map" << endl;
    BGL_FORALL_VERTICES(vertex, subGraph, Graph) {
        cout << subGraph[vertex].name << " " << boost::get(v_index_map, vertex) << endl;
    }

    cout << "Vertices in this->subGraph | init BC()" << endl;
    BGL_FORALL_VERTICES(vertex, this->subGraph, Graph) {
        cout << vertex << " " << this->subGraph[vertex].id << endl;
    }


}

void SubComponent::findBetweennessCentrality() {
    cout << "****** find BC() *******" << endl;
    cout << "Test art_points" << endl;
    for (VertexVecIter vi = art_points.begin(); vi != art_points.end(); ++vi) {
        cout << *vi << " ";
        cout << subGraph[*vi].name << endl;
    }

    cout << "Vertices in this->subGraph | find BC()" << endl;
    BGL_FORALL_VERTICES(vertex, this->subGraph, Graph) {
        cout << vertex << " " << this->subGraph[vertex].id << endl;
    }

    cout << "Test normal_vertices 2" << endl;
    for (vector<Vertex>::iterator vi = normal_vertices.begin(); vi != normal_vertices.end(); ++vi) {
        cout << *vi << " " << endl;
        cout << subGraph[*vi].name << endl;
    }
    cout << "BC abc " << endl;
    /* Test v_index_map */
    // BGL_FORALL_VERTICES(vertex, subGraph, Graph) {
    //     cout << subGraph[vertex].name << " " << boost::get(v_index_map, vertex) << endl;
    // }

    // /* Test v_centrality_map
    // **
    // */
    // cout << "v_centrality_map" << endl;
    // BGL_FORALL_VERTICES(vertex, subGraph, Graph) {
    //     cout << subGraph[vertex].name << endl;
    //     // double score = boost::get(v_centrality_map, vertex);
    //     // cout << vertex << " " << score << endl;
    // }

    // brandes_betweenness_centrality(subGraph, boost::centrality_map(v_centrality_map).vertex_index_map(v_index_map));
    cout << "BC done" << endl;
}

void SubComponent::printBetweennessCentrality() {
    cout << "Vertex betweenness" << endl;

    Viter vi, vi_end;
    int i = 0;
    for (boost::tie(vi, vi_end) = boost::vertices(subGraph); vi != vi_end; ++vi) {
        cout << subGraph[*vi].id << "\t" << v_centrality_vec.at(i) << endl;
        ++i;
    }
}
