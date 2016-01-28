//
// Created by quynh on 12/13/15.
//

#include "parser.h"

template<typename NameVertexMap>
void addLinkToGraph(string s1, string s2, double cost, Graph &g, NameVertexMap &routers) {
    // TODO: change routers --> routers_map

    Vertex v1, v2;
    Edge e;

    typename NameVertexMap::iterator pos;
    bool inserted;

    boost::tie(pos, inserted) = routers.insert(std::make_pair(s1, Vertex()));
    if (inserted) {
        v1 = boost::add_vertex(g);
        routers[s1] = v1;
        pos->second = v1;
        g[v1].id = s1;
        g[v1].label = s1;
    } else {
        v1 = pos->second;
    }

    boost::tie(pos, inserted) = routers.insert(std::make_pair(s2, Vertex()));
    if (inserted) {
        v2 = boost::add_vertex(g);
        routers[s2] = v2;
        pos->second = v2;
        g[v2].id = s2;
        g[v2].label = s2;
    } else {
        v2 = pos->second;
    }

    // Add edge (aka. link) connecting 2 vertices
    boost::tie(e, inserted) = boost::add_edge(v1, v2, g);
    if (inserted) {
        g[e].cost = cost;
    }
}

void readEdgeFile(string filePath, Graph &g) {
    // NameVertexMap is to keep track of which router has already been added
    typedef std::map<std::string, Vertex> NameVertexMap;
    NameVertexMap routers;

    ifstream inFile(filePath.c_str());

    vector<string> strs;
    for (string line; getline(inFile, line); /**/) {
        boost::split(strs, line, boost::is_any_of(" "));

        // Cast vector<string> to array<string, 3>
        // TODO: this is really crude way to do it.
        // TODO: how to copy some element of vector to array
        if (strs.size() == 3) {
            string source = strs[0];
            string target = strs[1];
            // TODO: use atof as a way around the error: ‘stof’ was not declared in this scope
            // double cost = stof(strs[2]);
            double cost = atof(strs[2].c_str());

            addLinkToGraph(source, target, cost, g, routers);

        }
    }
    inFile.close();
}

void readJson(string filePath, Graph &g) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(filePath, pt);

    // NameVertexMap is to keep track of which router has already been added
    typedef std::map<std::string, Vertex> NameVertexMap;
    NameVertexMap routers;

    BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, pt.get_child("links")) {
        cout << "X" << endl;
        cout << v.second.get_value<std::string>() << " ";
        string source = v.second.get_child("source").get_value<std::string>();
        string target = v.second.get_child("target").get_value<std::string>();
        double cost = v.second.get_child("cost").get_value<double>();


        addLinkToGraph(source, target, cost, g, routers);
    }
}

void readComplexJson(string filePath, Graph &g) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(filePath, pt);

    // NameVertexMap is to keep track of which router has already been added
    typedef std::map<std::string, Vertex> NameVertexMap;
    NameVertexMap routers;

    BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, pt.get_child("topology")) {
        cout << "X" << endl;
        cout << v.second.get_value<std::string>() << " ";
        string source = v.second.get_child("lastHopIP").get_value<std::string>();
        string target = v.second.get_child("destinationIP").get_value<std::string>();
        double cost = v.second.get_child("neighborLinkQuality").get_value<double>();

        addLinkToGraph(source, target, cost, g, routers);
    }
}

void readEdgeFileGraphManager(string filePath, GraphManager &gm) {
    // NameVertexMap is to keep track of which router has already been added
    ifstream inFile(filePath.c_str());

    vector<string> strs;
    for (string line; getline(inFile, line); /**/) {
        boost::split(strs, line, boost::is_any_of(" "));

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
    inFile.close();
}