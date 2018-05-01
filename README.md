# What is Prince:

[![Build Status](https://travis-ci.org/AdvancedNetworkingSystems/prince.svg?branch=master)](https://travis-ci.org/AdvancedNetworkingSystems/prince)

Prince is an open source implementation of the PopRouting Algorithm.
It has been developed as a Google Summer of Code
[Project](https://summerofcode.withgoogle.com/archive/2016/projects/5374689325088768/)
in collaboration with the University of Trento. PopRouting is an algorithm
to calculate the optimal values for the messages timers of a link state
routing protocol (OLSR, OSPF, etc). It fetches the topology data from the
routing protocol, calculate the betweenness centrality for every node of
the network and then push back the new timer's value. Currently supports OONF and OLSRd.

## Using Prince on Openwrt / LEDE

Since v0.2 prince can be found in the LEDE/OpenWRT routing feeds.

```bash
opkg update
opkg install prince
```

## Using Prince with OONF

Prince requires these plugins to work: `remotecontrol, telnet, netjsoninfo`.

This is a configuration example for OONF that works with prince:

```
[global]
      plugin remotecontrol

[log]
        stderr false
        file /var/log/olsrd2.log

[telnet]
	bindto	127.0.0.1
	allowed session 10

[remotecontrol]
	acl	default_accept

[interface=wlan0]
[interface=lo]
```

## Using Prince with OLSRd

Prince requires these plugins to work: `jsoninfo, poprouting`.

This is a configuration example for OLSRd that works with prince:

```
DebugLevel  1
IpVersion 4
FIBMetric "flat"
LinkQualityFishEye  0


LoadPlugin "../olsrd/lib/jsoninfo/olsrd_jsoninfo.so.1.1"{
    PlParam "accept" "0.0.0.0"
    PlParam "port" "2009"
}

LoadPlugin "../olsrd/lib/poprouting/olsrd_poprouting.so.1.0"{
    PlParam "accept" "0.0.0.0"
    PlParam "port" "2008"
}

InterfaceDefaults {
    TcInterval 5.0
    HelloInterval   2.0
}

Interface "wlan0" {}
```
NB: LinkQualityFishEye must be set to 0 in order to PopRouting to works

## Prince Configuration
Prince can be configured with a configuration file, this is an example:

```json
{
  "proto":{
    "protocol": "oonf",
    "host": "127.0.0.1",
    "port": 2009,
    "timer_port": 2009,
    "refresh": 1,
    "log_file" : "",
    "json_type": "netjson"

  },
  "graph-parser":{
    "heuristic": 1,
    "weights": 0,
    "recursive": 1,
    "stop_unchanged": 0,
    "multithreaded": 1
  }
}
```
### Proto
`protocol` specifies whether prince should interact with the routing daemon `olsr` or `oonf`.

`host` specify the address to reach the routing daemon, usually is localhost (127.0.0.1)

`port` is used to specify the port from where prince will pull the topology data

`timer_port` is used to specify the port to where prince will push the updated timers.

`refresh` specify the refresh rate at which prince will calculate the updated timers.

`log_file` is used to benchmark PRINCE and to check if the
timers and the centrality are correct, do not use it in production.

`json_type` is used to specify which format of json topology
we are parsing. It is used only by OLSRv1 plugin, supported values are: `netjson` or `jsoninfo`.

### Graph Parser

`heuristic` specify whether graph_parser should use the heuristic described in this paper to optimize the centrality computation

`weights` specify whether graph_parser should use weights in the centrality computation

`recursive`

`stop_unchanged`

`multithreaded` specify whether graph parser should use multiple thread to compute the centrality

## Development

Install the dependencies:

```bash
apt-get install libjson-c-dev
```
Make a build directory in the repo

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

### Debug

Set the build type to debug, this will add gdb's debug symbols to the binary

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Release

Set the build type to release to ask the compiler for more error checking

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

You can include gdb's debug symbols in a release using

```bash
cmake .. -DCMAKE_BUILD_TYPE=ReleaseWithDebug
```
