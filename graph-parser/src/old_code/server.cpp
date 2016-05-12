#include <cstdlib>
#include <iostream>
#include <string>
#include "parser.h"
#include "utility.h"

//gabriel
#include <cstring>      // Needed for memset
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>      // Needed for the socket functions

void runlistener(int format,string remote_host,int port){
  //single threaded server mode
  //when the server receives a request from a client, it calculates the Betweenness and return netjson_bc

  int status;
  struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
  struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
  memset(&host_info, 0, sizeof host_info); //erase hostinfo mem location

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, "2010", &host_info, &host_info_list);
  status = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1)  std::cerr << "bind error" << std::endl ;

  status =  listen(socketfd, 5);
  if (status == -1)  std::cerr << "listen error" << std::endl ;

  while(true){
    int client_sd;
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    client_sd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);
    if(client_sd==-1){
      std::cerr << "accept error" << std:endl;
    }
     //connect to olsr to download json
     pt:ptree json;
     json = get_json(remote_host, port)

     //compute BC

     //add BC to json


     // send json with bc back to client

  }
}

boost::property_tree::ptree get_json(string remote_host, int port){
  int server_sd, status;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;

  memset(&host_info, 0, sizeof host_info);
  host_info.ai_family = AF_UNSPEC;     // IP version not specified. Can be both.
  host_info.ai_socktype = SOCK_STREAM;
  status = getaddrinfo(remote_host, port, &host_info, &host_info_list);
  server_sd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                      host_info_list->ai_protocol);

  if (server_sd == -1)  std::cerr << "socket error ";

  status = connect(server_sd, host_info_list->ai_addr, host_info_list->ai_addrlen);

  ssize_t bytes_recieved;
  char incomming_data_buffer[1000];
  bytes_recieved = recv(server_sd, incomming_data_buffer,1000, 0);
  if (bytes_recieved == 0) std::cout << "host shut down." << std::endl ;
  if (bytes_recieved == -1)std::cerr << "recieve error!" << std::endl ;

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

int main(int argc, char * argv[]) {
  //arguments format:
  // 1st argument: remote topology format 1Netjson 2jsoninfo
  // 2nd : remote ip
  // 3rd : remote port
  int format=1; port=2007;
  string remote_host="127.0.0.1";
  if(argc >= 2){
    format = atoi(argv[1]);
  }
  if(argc >= 4 ) {
    remote_host = string(argv[2]);
    port = atoi(argv[3]);

  }

  runlistener(format, remote_host, port);


}

