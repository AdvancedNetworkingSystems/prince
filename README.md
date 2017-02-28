#What is Prince:
<<<<<<< HEAD
Prince is an open source implementation of the PopRouting Algorithm. 
PopRouting is an algorithm to calculate the optimal values for the messages timers of a link state routing protocol (OLSR, OSPF, etc). 
It fetch the topology data from the routing protcol, calculate the betweenness centrality for every node of the network and then push back the new timer's value. Currently it only supports OONF.

#Specific
This repo is a fork of https://github.com/gabri94/poprouting, in which we want to rewrite in C the C++ section, in order to reduce the final executable size. This allows it to run on smaller devices with lower computational power.

#How to Run
If you use the main Makefile present in the base directory, the result will be a file named "prince_c". You have to run it, specyfing a configuration file, like the one for C present in input folder (e.g. "prince/input/test_c.ini").

#How To Compile and Run on OpenWRT and Qemu
1. download latest stable versione of openWRT (e.g. git clone -b chaos_calmer git://github.com/openwrt/openwrt.git)
2. create a folder named openwrt/package/poprouting/ and copy there the Makefile.openwrt, named simply Makefile
3. if you issue "./scripts/feeds install poprouting", it will prepare the package and make it available
4. type "make menuconfig", choose "ARM Ltd. Realview board(qemu)" as Target System and check under Network/Routing and Redirection the entry named "poprouting". Make sure under Base System that "libpthread" is included (it should be by default).
5. send "make" command, this step will take up to 1.5 h the first time
6. start qemu with "qemu-system-arm -M realview-eb-mpcore -kernel ~/Downloads/openwrt/bin/realview/openwrt-realview-vmlinux-initramfs.elf -nographic -net user,hostfwd=tcp::10022-:22 -net nic" (port 10022 will be our ssh target)
7. inside qemu change root password to allow ssh coonection, using "passwd"
8. outside qemu, ssh into it with "ssh root@localhost -p 10022" (it may be necessary to start dhcp on related vlan, "udhcpc -i br-lan")
9. now you can copy the package inside qemu using "scp -P 10022 ~/Downloads/openwrt/bin/realview/packages/base/poprouting_master_realview.ipk  root@localhost:/"
10. inside qemu again, you should find the package and install it via " opkg install poprouting_master_realview.ipk".
11. If the following message appears, "verify_pkg_installable: Only have 0kb available on filesystem /overlay" while installing the package for disk space, change in /etc/opkg.conf the line "option overlay_root /overlay"  is the problem. Type "df -h *" to find a proper space for packages and then change the line accordingly, e.g "option overlay_root /tmp". Return to point 10.
12. Now you should provide a config file for the executable, copy it from host to qemu using "scp -P 10022 prince/input/test_c.ini  root@localhost:/" inside the project folder.
13. Now that the package is properly installed, you can run it using the command "prince test_c.ini &" in qemu.


=======
Prince is an open source implementation of the PopRouting Algorithm. It has been developed as a Google Summer of Code [Project](https://summerofcode.withgoogle.com/projects/#5453035123769344) in collaboration with the University of Trento.
PopRouting is an algorithm to calculate the optimal values for the messages timers of a link state routing protocol (OLSR, OSPF, etc).
It fetch the topology data from the routing protocol, calculate the betweenness centrality for every node of the network and then push back the new timer's value. Currently it only supports OONF.


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
`make && make install`



##Openwrt / LEDE
To use poprouting with OpenWRT you need to build yourself the openwrt image with the toolchain.
Create a folder in the openwrt toolchain root:
`mkdir package/poprouting`

Copy the makefile for OpenWRT from the git root to the new folder
`cp ../poprouting/Makefile.openwrt package/poprouting/Makefile`

Update the feeds:
`./scripts/feeds install poprouting`

Then run `make menuconfig` select the package and build the image

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
>>>>>>> upstream/master
