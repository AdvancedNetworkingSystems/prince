import re
import socket
import json
import numpy as np
from poprouting import ComputeTheoreticalValues
from graph_generator import Gen


class PrinceTestOONF:
    '''
    Generate a topology in netjson format and send it to a client
    type: type of topology generated: 0 CN, 1 PLAW, 2 MESH
    cs: clientsocket object
    N: nubmer of nodes of the topology
    '''
    def genG(self, type, cs, N):
        ''' type = 0    CN
            type = 1    PLAW
            type = 2    MESH
        '''
        T = int(N * 1.2)
        E = N * 1.8
        ge = Gen()
        if type == 0:
            ge.genCNGraph(N)
        if type == 1:
            ge.genGraph("PLAW", N)
        if type == 2:
            ge.genMeshGraph(N, 23)
        self.graph = ge.graph
        self.netjson = ge.composeNetJson(self.graph)
        self.json_netjson = json.dumps(self.netjson)
        self.h_py, self.tc_py = self.calculateTimers()
        # print ("sending graph")
        cs.send(self.json_netjson)
        cs.close()

    '''
    Calculate timers using the poprouting function CalculateTheoreticalValues
    and choose the one that matches our "router_id"
    '''
    def calculateTimers(self):
        # compute the timers values using the poprouting python lib
        ctv = ComputeTheoreticalValues(graph=self.graph)
        hello_py = tc_py = 0
        # search for the timer we are calculating (the 'router_id' one)
        for node in ctv.node_list:
            if node == self.netjson['router_id']:
                hello_py = ctv.Hi[node]
                tc_py = ctv.TCi[node]
        return hello_py, tc_py
    '''
    Run a test with the heuristic
    type: see below
    N: number of nodes
    iter: number of batch tests
    '''
    def test(self, type, N, iter, port):
        ''' listen to the port 2009 to emulate a oonf's daemon.
            when prince connect it reply with the netjson and then wait for the optimized timers
        '''
        serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        serversocket.bind(('0.0.0.0', port))
        serversocket.listen(5)
        # regular expression to catch the timers' values
        p = re.compile(r"\d*\.\d+")
        tc_cpp = 0
        executions = []
        hs = []
        tcs = []
        while iter:
            # accept connections
            (cs, address) = serversocket.accept()
            data = cs.recv(1024).decode("utf-8")
            # simulate the oonf's netjson command to get the topology
            if data.strip() == "/netjsoninfo filter graph ipv6_0/quit":
                # generate the graph, calculate timers(python) and push topology to prince
                self.genG(type, cs, N)

            else:
                # search for the timers' values using the regex
                if data:
                    toks = p.findall(data)
                    if toks:
                        tc_cpp = float(toks[0])
                        hello_cpp = float(toks[1])
                        exec_time = float(toks[2])
                        # calculate the percentage error between the reference (py) and the measured (C[++]) values
                        tc_err = abs(tc_cpp - self.tc_py) / ((tc_cpp + self.tc_py) / 2) * 100
                        h_err = abs(hello_cpp - self.h_py) / ((hello_cpp + self.h_py) / 2) * 100
                        hs.append(h_err)
                        tcs.append(tc_err)
                        executions.append(exec_time)
                        cs.close()
                        iter -= 1
        executions = np.array(executions)
        hs = np.array(hs)
        tcs = np.array(tcs)
        mat = np.dstack((executions, hs, tcs)).squeeze()
        return mat
