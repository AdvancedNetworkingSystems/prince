import re,socket
from netdiff import NetJsonParser
import networkx as nx
import subprocess
import random
import json
from poprouting import ComputeTheoreticalValues
from graph_generator import Gen

class PrinceTestOONF:
    def calculateTimers(self):
        #compute the timers values using the poprouting python lib
        ctv = ComputeTheoreticalValues(graph = self.graph)
        #search for the timer we are calculating (the 'router_id' one)
        for node in ctv.node_list:
        	if node == self.netjson['router_id']:
        		hello_py = ctv.Hi[node]
        		tc_py = ctv.TCi[node]
        return hello_py, tc_py

    def test(self, type):
        ''' type = 0    CN
            type = 1    PLAW
            type = 2    MESH
        '''
        ''' listen to the port 2009 to emulate a oonf's daemon.
            when prince connect it reply with the netjson and then wait for the optimized timers
        '''
        serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        serversocket.bind(('localhost', 2009))
        serversocket.listen(5)
        #regular expression to catch the timers' values
        p=re.compile(r"\d*\.\d+")
        #subprocess.Popen("../../build/prince ../../input/test.ini", shell=True)
        tc_cpp = 0
        while not tc_cpp:
            #accept connections
            (cs, address) = serversocket.accept()
            data = cs.recv(1024).decode("utf-8")
            #simulate the oonf's netjson command to get the topology
            if data.strip() == "/netjsoninfo filter graph ipv6_0/quit":
                N=100
                T=150
                E=250
                ge = Gen()
                if type == 0:
                    ge.genCN(N,T,E,S=4)
                if type == 1:
                    ge.genGraph("PLAW", N)
                if type == 2:
                    ge.genMeshGraph(N, 23)
                self.graph = ge.graph
                self.netjson =  ge.composeNetJson(self.graph)
                self.json_netjson = json.dumps(self.netjson)
                h_py, tc_py = self.calculateTimers()
                print ("sending graph")
                print cs.send(self.json_netjson)
                cs.close()

            else:
                #search for the timers' values using the regex
                if data:
                    toks = p.findall(data)
                    if toks:
                        tc_cpp = float(toks[0])
                        hello_cpp = float(toks[1])
                        p_tc = abs(tc_cpp - tc_py)/((tc_cpp+ tc_py)/2)*100
                        p_h = abs(hello_cpp - h_py)/((hello_cpp+ h_py)/2)*100
                        print "node: " + str(self.netjson['router_id'])
                        print "       " + "C++  " + "Python" + "              " + "Percent"
                        print "tc:    " + repr(tc_cpp) + "   " + repr(tc_py) + " " +  str(p_tc)
                        print "hello: " + repr(hello_cpp) + "   " + repr(h_py) + " " + str(p_h)
                        cs.close()
                        tc_cpp = 0
p = PrinceTestOONF()
p.test(2)
