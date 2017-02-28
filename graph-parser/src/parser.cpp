//
// Created by quynh on 12/13/15.
//

#include "parser.h"



 void parse_simplegraph(struct topology *topo,  GraphManager &gm){
	 struct node *punt;
   gm.ResetVerticesAndEdgesIndexMap();
	 for(punt=topo->first; punt!=0; punt=punt->next){
		 struct neighbor* neigh;
		 for(neigh=punt->neighbor_list; neigh!=0; neigh=neigh->next){
			 string source = punt->id;
			 string target = neigh->id->id;
			 double cost = neigh->weight;

			 GraphManager::VertexProperties vp1 = GraphManager::VertexProperties(source, source);
			 GraphManager::VertexProperties vp2 = GraphManager::VertexProperties(target, target);
			 GraphManager::EdgeProperties ep = GraphManager::EdgeProperties(cost);
			 gm.AddEdge(vp1, vp2, ep);
			 //cout << source << " " << target << " " << cost << endl;
		 }
	 }

 }

 void parse_netjson(std::basic_istream<char> & istream, GraphManager &gm){
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(istream, pt);

    BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, pt.get_child("links")) {
        //cout << v.second.get_value<std::string>() << " ";
        string source = v.second.get_child("source").get_value<std::string>();
        string target = v.second.get_child("target").get_value<std::string>();
        double cost = v.second.get_child("cost").get_value<double>();

        GraphManager::VertexProperties vp1 = GraphManager::VertexProperties(source, source);
        GraphManager::VertexProperties vp2 = GraphManager::VertexProperties(target, target);
        GraphManager::EdgeProperties ep = GraphManager::EdgeProperties(cost);
        gm.AddEdge(vp1, vp2, ep);
    }
  }

 void parse_jsoninfo(std::basic_istream<char> & istream, GraphManager &gm){
	 boost::property_tree::ptree pt;
	     boost::property_tree::read_json(istream, pt);

	     BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, pt.get_child("topology")) {
	         //cout << v.second.get_value<std::string>() << " ";
	         string source = v.second.get_child("lastHopIP").get_value<std::string>();
	         string target = v.second.get_child("destinationIP").get_value<std::string>();
	         double cost = v.second.get_child("neighborLinkQuality").get_value<double>();

	         GraphManager::VertexProperties vp1 = GraphManager::VertexProperties(source, source);
	         GraphManager::VertexProperties vp2 = GraphManager::VertexProperties(target, target);
	         GraphManager::EdgeProperties ep = GraphManager::EdgeProperties(cost);
	         gm.AddEdge(vp1, vp2, ep);
	     }
 }
