#What is Prince:
Prince is an open source implementation of the PopRouting Algorithm. It has been developed as a Google Summer of Code [Project](https://summerofcode.withgoogle.com/projects/#5453035123769344) in collaboration with the University of Trento.
PopRouting is an algorithm to calculate the optimal values for the messages timers of a link state routing protocol (OLSR, OSPF, etc).
It fetch the topology data from the routing protocol, calculate the betweenness centrality for every node of the network and then push back the new timer's value. Currently it only supports OONF.


#How to install it:
##Linux
Dependencies: libjson-c

Clone the repository in your pc with :
`git clone https://github.com/gabri94/poprouting.git`

Enter in the directory :
`cd poprouting`

Compile and install:
`make && make install`



##Openwrt / LEDE
Since v0.2 prince is in the LEDE/OpenWRT routing feeds.
You just need to type:
opkg update
opkg install prince

#How to use it
## Prince Configuration
Prince can be configured with a configuration file, this is an example:
```
[proto]
protocol = oonf
host  = 127.0.0.1
port = 2009
refresh = 1

[graph-parser]
heuristic  = 1
weights  = 0

```


##OONF
To use Prince with OONF you need the following plugins: `remotecontrol, telnet, netjsoninfo`.

This is a configuration example that works with prince:
```
[global]
      plugin remotecontrol

[olsrv2]
      originator    -0.0.0.0/0
      originator    -::1/128
      originator    default_accept

[log]
        stderr false
        file /var/log/olsrd2.log
        info all

[interface]
        bindto        -0.0.0.0/0
        bindto        -::1/128
        bindto        default_accept
[telnet]
	bindto	127.0.0.1
	allowed session 10

[remotecontrol]
	acl	default_accept

[interface=wlan0]
[interface=lo]
```
