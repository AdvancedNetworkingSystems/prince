#What is Prince:
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
3. if you issue "./scripts/feeds install poprouting", it will prepare the package and made it available
4. type "make menuconfig", choose "ARM Ltd. Realview board(qemu)" as Target System and check under Network/Routing and Redirection the entry named "poprouting"
5. send "make" command, this step will take up to 1.5 h the first time
6. start qemu with "qemu-system-arm -M realview-eb-mpcore -kernel ~/Downloads/openwrt/bin/realview/openwrt-realview-vmlinux-initramfs.elf -nographic -net user,hostfwd=tcp::10022-:22 -net nic" (port 10022 will be our ssh target"
7. inside qemu change root password to allow ssh coonection, using "passwd"
8. outside qemu, ssh into it with "ssh root@localhost -p 10022" (it may be necessary to start dhcp on related vlan, "udhcpc -i br-lan")
9. now you can copy the package inside qemu using "scp -P 10022 ~/Downloads/openwrt/bin/realview/packages/base/poprouting_master_realview.ipk  root@localhost:/"
10. inside qemu again, you should find the package and install it via " opkg install poprouting_master_realview.ipk".
11. If the following message appears, "verify_pkg_installable: Only have 0kb available on filesystem /overlay" while installing the package for disk space, change in /etc/opkg.conf the line "option overlay_root /overlay"  is the problem. Type "df -h *" to find a proper space for packages and then change the line accordingly, e.g "option overlay_root /tmp". Return to point 10.
12. Now you should provide a config file for the executable, copy it from host to qemu using "scp -P 10022 prince/input/test_c.ini  root@localhost:/" inside the project folder.
13. Now that the package is properly installed, you can run it using the command "prince test_c.ini &" in qemu.


