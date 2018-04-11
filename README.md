# What is Prince:

Prince is an open source implementation of the PopRouting Algorithm.
It has been developed as a Google Summer of Code
[Project](https://summerofcode.withgoogle.com/projects/#5453035123769344)
in collaboration with the University of Trento. PopRouting is an algorithm
to calculate the optimal values for the messages timers of a link state
routing protocol (OLSR, OSPF, etc). It fetches the topology data from the
routing protocol, calculate the betweenness centrality for every node of
the network and then push back the new timer's value. Currently it only supports OONF.

## Using Prince on Openwrt / LEDE

Since v0.2 prince is in the LEDE/OpenWRT routing feeds.

```bash
opkg update
opkg install prince
```

## Using Prince with OONF

Prince requires these plugins to work: `remotecontrol, telnet, netjsoninfo`.

This is a configuration example for OONF that works with prince:

```ini
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
    "json_type": "netjson"
    "log_file" : ""

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

`log_file` is used to benchmark PRINCE and to check if the
timers and the centrality are correct, do not use it in production.

`json_type` is used to specify which format of json topology
we are parsing. It is used only by OLSRv1 plugin, supported values are: `netjson` or `jsoninfo`.

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
