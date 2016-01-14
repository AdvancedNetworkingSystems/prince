//
// Created by quynh on 12/15/15.
//

#include "centrality.h"

void simpleBetweennessCentrality(const Graph &g, string fileSuffix) {
    // One way to create centrality_map
    //    boost::shared_array_property_map<double, boost::property_map<Graph, vertex_index_t>::const_type>
    //            centrality_map(num_vertices(g), get(boost::vertex_index, g));


    // Define VertexCentralityMap
//    typedef boost::property_map< Graph, boost::vertex_index_t>::type VertexIndexMap;
//    VertexIndexMap v_index = get(boost::vertex_index, g);
//    // Constructs a container with n elements. Each element is a copy of val.
//    std::vector< double > v_centrality_vec(boost::num_vertices(g), 0.0);
//    // Create the external property map
//    boost::iterator_property_map< std::vector< double >::iterator, VertexIndexMap >
//            v_centrality_map(v_centrality_vec.begin(), v_index);
//
//    brandes_betweenness_centrality( g, v_centrality_map);


    // Nov 20, 2015
    // http://stackoverflow.com/questions/15432104/how-to-create-a-propertymap-for-a-boost-graph-using-lists-as-vertex-container

    StdVertexIndexMap v_index_std_map;
    VertexIndexMap v_index_map(v_index_std_map);

    // Populate the indexMap
    Viter vi, ve; // vertex_iterator, vertex_iterator_end
    size_t i = 0;
    for (boost::tie(vi, ve) = boost::vertices(g); vi != ve; ++vi) {
        boost::put(v_index_map, *vi, i);
        ++i;
    }

    typedef std::vector<double> CentralityVec;
    CentralityVec v_centrality_vec(boost::num_vertices(g), 0);

    typedef boost::iterator_property_map<CentralityVec::iterator, VertexIndexMap> CentralityMap;
    CentralityMap v_centrality_map(v_centrality_vec.begin(), v_index_map);

    // Nov 26, try out the normal call to centrality().This version is not working.
//    brandes_betweenness_centrality( g, boost::centrality_map(v_centrality_map));

    // http://stackoverflow.com/questions/30263594/adding-a-vertex-index-to-lists-graph-on-the-fly-for-betweenness-centrality
    // Use named-parameter
    brandes_betweenness_centrality(g, boost::centrality_map(v_centrality_map).vertex_index_map(v_index_map));
    relative_betweenness_centrality(g, v_centrality_map);


    // Print result of v_centrality_map to console
    cout << "Vertex betweenness" << endl;
    i = 0;
    for (boost::tie(vi, ve) = boost::vertices(g); vi != ve; ++vi) {
        cout << g[*vi].id << "\t" << v_centrality_vec.at(i) << endl;
        ++i;
    }

    // Write result of v_centrality_map to file.
    writeBetweennessCentrality(g, v_centrality_vec, fileSuffix);

}


void writeBetweennessCentrality(const Graph &g, std::vector<double> v_centrality_vec, string fileSuffix) {
    cout << "XXX Writing to File";
    string filePath = "../output/boost_" + fileSuffix + ".csv";
    ofstream outFile(filePath.c_str());

    Viter vi, ve;
    size_t i = 0;
    if (outFile.is_open()) {
        for (boost::tie(vi, ve) = boost::vertices(g); vi != ve; ++vi) {
            outFile << g[*vi].id << ", " << v_centrality_vec.at(i) << endl;
            ++i;
        }
    }
    outFile.close();

    cout << "XXX Writing to File 2";
}
