#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_graph.hpp>// A subclass to provide reasonable arguments to adjacency_list for a typical undirected graph
#include <boost/graph/betweenness_centrality.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace std;


struct Router {
    string id;
    string name;

    Router() { };

    Router(string id, string name) : id(id), name(name) { }
};

struct Link {
    double cost;

    Link() { };

    Link(double cost) : cost(cost) { };
};

typedef std::array<string, 3> graphDataType;
typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS,
        Router, Link> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::graph_traits<Graph>::vertex_iterator Viter;

typedef boost::graph_traits<Graph>::edge_descriptor Edge;

vector<vector<string>> readFile1(string filePath) {
    ifstream inFile(filePath);

    // Reading to vector<string>
    vector<vector<string>> contents;
    vector<string> strs;
    for (string line; getline(inFile, line); /**/) {
        boost::split(strs, line, boost::is_any_of(" "));
        contents.push_back(strs);
    }
    inFile.close();

    return contents;
}

vector<graphDataType> readFile2(string filePath) {
    ifstream inFile(filePath);

    // Reading to vector<graphDataType>
    vector<graphDataType> contents;
    vector<string> strs;
    graphDataType graph_strs;
    for (string line; getline(inFile, line); /**/) {
        boost::split(strs, line, boost::is_any_of(" "));

        // Cast vector<string> to array<string, 3>
        // TODO: this is really crude way to do it.
        // TODO: how to copy some element of vector to array
        vector<string>::iterator i = strs.begin();
        for (int i = 0; i < strs.size(); ++i) {
            graph_strs[i] = strs[i];
        }
        contents.push_back(graph_strs);
    }
    inFile.close();

    return contents;

}

template <typename NameVertexMap>
void addLinkToGraph(string s1, string s2, double cost, Graph& g, NameVertexMap& routers) {
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
        g[v1].name = s1;
    } else {
        v1 = pos->second;
    }

    boost::tie(pos, inserted) = routers.insert(std::make_pair(s2, Vertex()));
    if (inserted) {
        v2 = boost::add_vertex(g);
        routers[s2] = v2;
        pos->second = v2;
        g[v2].id = s2;
        g[v2].name = s2;
    } else {
        v2 = pos->second;
    }

    // Add edge (aka. link) connecting 2 vertices
    boost::tie(e, inserted) = boost::add_edge(v1, v2, g);
    if (inserted) {
        g[e].cost = cost;
    }
}

void readJson(string filePath, Graph& g) {
    vector<graphDataType> contents;
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(filePath, pt);

    Vertex v1, v2;
    Edge e;

    // NameVertexMap is to keep track of which router has already been added
    typedef std::map<std::string, Vertex> NameVertexMap;
    NameVertexMap routers;

    BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, pt.get_child("links"))
    {
        cout << "X" << endl;
        cout << v.second.get_value<std::string>() << " ";
        string source = v.second.get_child("source").get_value<std::string>();
        string target = v.second.get_child("target").get_value<std::string>();
        double cost = v.second.get_child("cost").get_value<double>();


        addLinkToGraph(source, target, cost, g, routers);
    }
}

void printVector1(vector<vector<string>> contents) {
    vector<vector<string>>::iterator i;
    vector<string>::iterator j;

    int count = 0;
    for (i = contents.begin(); i != contents.end(); ++i) {
        for (j = (*i).begin(); j != (*i).end(); ++j) {
            cout << *j << endl;
        }
        cout << endl;
    }
}

void printVector2(vector<graphDataType> contents) {
    vector<graphDataType>::iterator i;
    int count = 0;
    for (i = contents.begin(); i != contents.end(); ++i) {
        unsigned long size = (*i).size();
//        cout << "size = " << size << endl;
//        cout << "&i = " << &i << endl;
        for (int j = 0; j < size; ++j) {
            cout << (*i)[j] << endl;
        }
        cout << endl;
    }
}


Vertex findRouter(string id, Graph g) {
    typedef pair<Viter, Viter> ViterPair;
    ViterPair vs = boost::vertices(g); // return the pointer at the beginning and at the end of the range.
    cout << &vs.first << "  ***  " << &vs.second << endl;
    Viter vit = vs.first;
    for (vit; vit != vs.second; ++vit) {
        cout << "+++ " << g[*vit].id << endl;
        cout << &vit << endl;
        cout << "Found " << id << endl;
        if (g[*vit].id == id) {

            return (*vit);
        }
    }
    cout << "Not Found " << id << endl;
    return NULL;
}

void createUndirectedGraph(vector<graphDataType> contents, Graph &g) {
    /*
     * Check the BGL Tutorial, pg. 64
    */
    Vertex v1, v2;
    Edge e;

    // NameVertexMap is to keep track of which router has already been added
    typedef std::map<std::string, Vertex> NameVertexMap;
    NameVertexMap routers;
    NameVertexMap::iterator pos;
    bool inserted;

    vector<graphDataType>::iterator iter;
    for (iter = contents.begin(); iter != contents.end(); ++iter) {
        // Get the id of 2 vertices (routers), and add them to graph
        if ((*iter).size() != 3) {
            cout << "XXX" << endl;
            continue;
        }
        string s1 = (*iter)[0];
        string s2 = (*iter)[1];
        float cost = std::stof((*iter)[2]);

        cout << s1 << " " << s2 << " " << cost << endl;

        boost::tie(pos, inserted) = routers.insert(std::make_pair(s1, Vertex()));
        if (inserted) {
//            cout << "Added vertex " << s1 << endl;
            v1 = boost::add_vertex(g);
            routers[s1] = v1;
            pos->second = v1;

            //Update the id and name
            g[v1].id = s1;
            g[v1].name = s1;
        } else {
//            cout << "Found vertex " << s1 << endl;
            v1 = pos->second;
        }

        boost::tie(pos, inserted) = routers.insert(std::make_pair(s2, Vertex()));
        if (inserted) {
//            cout << "Added vertex " << s2 << endl;
            v2 = boost::add_vertex(g);
            routers[s2] = v2;
            pos->second = v2;
            g[v2].id = s2;
            g[v2].name = s2;
        } else {
//            cout << "Found vertex " << s2 << endl;
            v2 = pos->second;
        }

        // Add edge (aka. link) connecting 2 vertices
        boost::tie(e, inserted) = boost::add_edge(v1, v2, g);
        if (inserted) {
            cout << "Added edge with weight " << cost << endl;
            g[e].cost = cost;
        }
    }
}

void printGraph(Graph g) {
    typename boost::graph_traits<Graph>::out_edge_iterator out_i, out_ie;
    Viter v_i, v_ie;

    Vertex v;
    Edge e;

    boost::tie(v_i, v_ie) = boost::vertices(g);
    for (v_i; v_i != v_ie; ++v_i) {
        v = *v_i;
        boost::tie(out_i, out_ie) = boost::out_edges(v, g);

        for (out_i; out_i != out_ie; ++out_i) {
            e = *out_i;
            Vertex src = boost::source(e, g);
            Vertex targ = boost::target(e, g);
            std::cout << "(" << g[src].id << ","
            << g[targ].id << ") ";
        }
        cout << endl;
    }
}

void handleSimpleInput(string filePath) {
    // Read the input.edges
    vector<graphDataType> contents;
    contents = readFile2(filePath);
    printVector2(contents);
    Graph g;
    createUndirectedGraph(contents, g);
    cout << "Finish creating graph" << endl;
    printGraph(g);
}


void simpleBetweennessCentrality(Graph g) {
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
    typedef std::map<Vertex, size_t> StdVertexIndexMap;
    StdVertexIndexMap idxMap;

    // This property map is an adaptor that converts any type that is a model of Unique Associative Container such as std::map into a mutable Lvalue Property Map.
    typedef boost::associative_property_map<StdVertexIndexMap> VertexIndexMap;
    VertexIndexMap v_index(idxMap);

    // Populate the indexMap
    Viter v_iter, v_iter_end;
    size_t i = 0;
    for (boost::tie(v_iter, v_iter_end) = boost::vertices(g); v_iter != v_iter_end; ++v_iter) {
        boost::put(v_index, *v_iter, i);
        ++i;
    }

    std::vector<double> v_centrality_vec(boost::num_vertices(g), 0);
    boost::iterator_property_map< std::vector< double >::iterator, VertexIndexMap >
            v_centrality_map(v_centrality_vec.begin(), v_index);

    // http://stackoverflow.com/questions/30263594/adding-a-vertex-index-to-lists-graph-on-the-fly-for-betweenness-centrality
    // Use named-parameter
    brandes_betweenness_centrality( g,
            boost::centrality_map(v_centrality_map).vertex_index_map(v_index));

    // Print result of v_centrality_map

    cout << "Vertex betweenness" << endl;
    i = 0;
    for(boost::tie(v_iter,v_iter_end) = boost::vertices(g); v_iter != v_iter_end; ++v_iter){
        cout << g[*v_iter].id << "\t" << v_centrality_vec.at(i) << endl;
        ++i;
    }

}

void handleJsonInput(string filePath) {
    Graph g;
    readJson(filePath, g);
    printGraph(g);

    // Applying the betweenness centrality
    simpleBetweennessCentrality(g);

    cout << "Done with Betweenness Centrality" << endl;
}

int main(int, char *[]) {
    string edgeFilePath = "../input/ninux_30_1.edges";
    //handleSimpleInput(edgeFilePath);

    string jsonFilePath = "../input/olsr-netjson.json";
    handleJsonInput(jsonFilePath);
    return 0;
}