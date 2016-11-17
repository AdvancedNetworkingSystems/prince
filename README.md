#What is Prince:
Prince is an open source implementation of the PopRouting Algorithm. It has been developed as a Google Summer of Code [Project](https://summerofcode.withgoogle.com/projects/#5453035123769344) in collaboration with the University of Trento. 
PopRouting is an algorithm to calculate the optimal values for the messages timers of a link state routing protocol (OLSR, OSPF, etc). 
It fetch the topology data from the routing protcol, calculate the betweenness centrality for every node of the network and then push back the new timer's value. Currently it only supports OONF.

#Specific
This repo is a fork of https://github.com/gabri94/poprouting, in which we want to rewrite in C the C++ section, in order to reduce the final executable size. This allows it to run on smaller devices with lower computational power.
