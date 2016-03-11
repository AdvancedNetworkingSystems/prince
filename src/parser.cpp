//
// Created by quynh on 12/13/15.
//

#include "parser.h"

void readEdgeFileGraphManager(string filepath, GraphManager &gm) {
    // NameVertexMap is to keep track of which router has already been added
    ifstream inFile(filepath.c_str());

    vector<string> strs;
    for (string line; getline(inFile, line); /**/) {
        boost::split(strs, line, boost::is_any_of(" ,"), boost::token_compress_on);

        // Cast vector<string> to array<string, 3>
        // TODO: this is really crude way to do it.
        // TODO: how to copy some element of vector to array
        if (strs.size() == 3) {
            string source = strs[0];
            string target = strs[1];

            GraphManager::VertexProperties vp1 = GraphManager::VertexProperties(source, source);
            GraphManager::VertexProperties vp2 = GraphManager::VertexProperties(target, target);

            // TODO: use atof as a way around the error: ‘stof’ was not declared in this scope
            // double cost = stof(strs[2]);
            double cost = atof(strs[2].c_str());

            GraphManager::EdgeProperties ep = GraphManager::EdgeProperties(cost);
            gm.AddEdge(vp1, vp2, ep);
        }
    }
    gm.ResetVerticesAndEdgesIndexMap();
    inFile.close();
}

void readJsonGraphManager(string filepath, GraphManager &gm) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(filepath, pt);

    BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, pt.get_child("links")) {
        cout << v.second.get_value<std::string>() << " ";
        string source = v.second.get_child("source").get_value<std::string>();
        string target = v.second.get_child("target").get_value<std::string>();
        double cost = v.second.get_child("cost").get_value<double>();

        GraphManager::VertexProperties vp1 = GraphManager::VertexProperties(source, source);
        GraphManager::VertexProperties vp2 = GraphManager::VertexProperties(target, target);
        GraphManager::EdgeProperties ep = GraphManager::EdgeProperties(cost);
        gm.AddEdge(vp1, vp2, ep);
    }
}

void readComplexJsonGraphManager(string filepath, GraphManager &gm) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(filepath, pt);

    BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, pt.get_child("topology")) {
        cout << v.second.get_value<std::string>() << " ";
        string source = v.second.get_child("lastHopIP").get_value<std::string>();
        string target = v.second.get_child("destinationIP").get_value<std::string>();
        double cost = v.second.get_child("neighborLinkQuality").get_value<double>();

        GraphManager::VertexProperties vp1 = GraphManager::VertexProperties(source, source);
        GraphManager::VertexProperties vp2 = GraphManager::VertexProperties(target, target);
        GraphManager::EdgeProperties ep = GraphManager::EdgeProperties(cost);
        gm.AddEdge(vp1, vp2, ep);
    }
}
