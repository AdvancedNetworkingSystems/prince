import re,socket
from netdiff import NetJsonParser
import networkx as nx
import subprocess
import random
import json
from poprouting import ComputeTheoreticalValues
from graph_generator import Gen

class PrinceTestOONF:
    def genG(self, type, cs, N):
        T=int(N*1.5)
        E=N*2
        ge = Gen()
        if type == 0:
            ge.genCNGraph(N,T,E,S=2)
        if type == 1:
            ge.genGraph("PLAW", N)
        if type == 2:
            ge.genMeshGraph(N, 23)
        self.graph = ge.graph
        self.netjson =  ge.composeNetJson(self.graph)
        self.json_netjson = json.dumps(self.netjson)
        self.h_py, self.tc_py = self.calculateTimers()
        print ("sending graph")
        print cs.send(self.json_netjson)
        cs.close()


    def calculateTimers(self):
        #compute the timers values using the poprouting python lib
        ctv = ComputeTheoreticalValues(graph = self.graph)
        hello_py = tc_py = 0;
        #search for the timer we are calculating (the 'router_id' one)
        for node in ctv.node_list:
        	if node == self.netjson['router_id']:
        		hello_py = ctv.Hi[node]
        		tc_py = ctv.TCi[node]
        return hello_py, tc_py

    def test(self, type, N, iter):
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
        while iter:
            #accept connections
            (cs, address) = serversocket.accept()
            data = cs.recv(1024).decode("utf-8")
            #simulate the oonf's netjson command to get the topology
            if data.strip() == "/netjsoninfo filter graph ipv6_0/quit":
                #generate the graph, calculate timers(python) and push topology to prince
                self.genG(type, cs, N)

            else:
                #search for the timers' values using the regex
                if data:
                    toks = p.findall(data)
                    if toks:
                        tc_cpp = float(toks[0])
                        hello_cpp = float(toks[1])
                        p_tc = abs(tc_cpp - self.tc_py)/((tc_cpp+ self.tc_py)/2)*100
                        p_h = abs(hello_cpp - self.h_py)/((hello_cpp+ self.h_py)/2)*100
                        print "node: " + str(self.netjson['router_id'])
                        print "       " + "C++  " + "Python" + "              " + "Percent"
                        print "tc:    " + repr(tc_cpp) + "   " + repr(self.tc_py) + " " +  str(p_tc)
                        print "hello: " + repr(hello_cpp) + "   " + repr(self.h_py) + " " + str(p_h)
                        cs.close()
                        iter = iter -1
                        if (p_tc > 1) or (p_h > 1):
                            print "Failed test"
                            exit(0)

p = PrinceTestOONF()
p.test(1, 50, 14)
