# What is Prince:

Prince is an open source implementation of the PopRouting Algorithm.
It has been developed as a Google Summer of Code
[Project](https://summerofcode.withgoogle.com/projects/#5453035123769344)
in collaboration with the University of Trento. PopRouting is an algorithm
to calculate the optimal values for the messages timers of a link state
routing protocol (OLSR, OSPF, etc). It fetches the topology data from the
routing protocol, calculate the betweenness centrality for every node of
the network and then push back the new timer's value. Currently it only supports OONF.


# How to install it:

## Linux
Install the dependencies:

```bash
apt-get install libjson-c-dev
```

Clone the repository in your pc with :

```
git clone https://github.com/AdvancedNetworkingSystems/poprouting/`
```

Enter in the directory :

```
cd poprouting
```

Make a build directory

```bash
mkdir build && cd build
```

Run cmake to generate build instructions

```bash
cmake ..
```

Compile and install:

```
make && make install
```

## Openwrt / LEDE
Since v0.2 prince is in the LEDE/OpenWRT routing feeds.
You just need to type:

```
opkg update
opkg install prince
```

# How to use it
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

`log_file` is used to benchmark PRINCE and to check if the timers and the centrality are correct, do not use it in production.

`json_type` is used to specify which format of json topology we are parsing. It is used only by OLSRv1 plugin, supported values are: `netjson` or `jsoninfo`.

## OONF

To use Prince with OONF you need the following plugins: `remotecontrol, telnet, netjsoninfo`.

This is a configuration example that works with prince:

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
