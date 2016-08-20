#What is Prince:
Prince is a tool used to calculate the optimal timer for a link state routing protocol. 

It fetch the topology data from the routing protcol, calculate the betweenness centrality for every node of the network and then push back the new timer's value.


#How to install it:
##Linux
Dependencies: libboost, libjson-c
Clone the repository in your pc with :
`git clone https://github.com/gabri94/poprouting.git`
Enter in the directory :
`cd poprouting`
Checkout the v0.1 :
`git checkout v0.1`
Compile and install:
`make`
`make install`


##Openwrt / LEDE
To use poprouting with OpenWRT you need to build yourself the openwrt image with the toolchain. 
Create a folder in the openwrt toolchain root:
`mkdir package/poprouting`
Copy the makefile for OpenWRT from the git root to the new folder
`cp ../poprouting/Makefile.openwrt package/poprouting/Makefile`

Update the feeds:
`./scripts/feeds install poprouting`

Then run `make menuconfig` select the package and build the image

