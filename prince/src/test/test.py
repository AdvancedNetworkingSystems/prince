'''
Copyright (c) 2016 Gabriele Gemmi <gabriel@autistici.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''
import re
import socket
import subprocess
import json
import numpy as np
from poprouting import ComputeTheoreticalValues
from graph_generator import Gen
import matplotlib.pyplot as plt
import sys


class PrinceTestOONF:
    '''
    Generate a topology in netjson format and send it to a client
    type: type of topology generated: 0 CN, 1 PLAW, 2 MESH
    cs: clientsocket object
    N: nubmer of nodes of the topology
    '''
    def genG(self, type, cs, N):
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
        print ("sending graph")
        print cs.send(self.json_netjson)
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

    def test_p(self, type, N, iter, port):
	''' type = 0    CN
            type = 1    PLAW
            type = 2    MESH
        '''
        ''' listen to the port 2009 to emulate a oonf's daemon.
            when prince connect it reply with the netjson and then wait for the optimized timers
        '''
        serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        serversocket.bind(('localhost', port))
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
                    print(toks)
                    if toks:
                        tc_cpp = float(toks[0])
                        hello_cpp = float(toks[1])
                        exec_time = float(toks[2])
                        p_tc = abs(tc_cpp - self.tc_py) / ((tc_cpp + self.tc_py) / 2) * 100
                        p_h = abs(hello_cpp - self.h_py) / ((hello_cpp + self.h_py) / 2) * 100
                        hs.append(p_h)
                        tcs.append(p_tc)
                        print port,"node: " + str(self.netjson['router_id'])
                        if port > 2015:
                            print "       " + "C  " + "Python" + "              " + "Percent"
                        else:
                            print "       " + "C++  " + "Python" + "              " + "Percent"
                        print "tc:    " + repr(tc_cpp) + "   " + repr(self.tc_py) + " " + str(p_tc)
                        print "hello: " + repr(hello_cpp) + "   " + repr(self.h_py) + " " + str(p_h)
                        executions.append(exec_time)
                        cs.close()
                        iter = iter - 1
        measures = {}
        measures['exec_mean'] = np.mean(executions)
        measures['exec_var'] = np.std(executions)
        measures['h_mean'] = np.mean(hs)
        measures['tc_mean'] = np.mean(tcs)
        measures['h_var'] = np.std(hs)
        measures['tc_var'] = np.std(tcs)
        print "Average execution time with " + str(N) + " nodes is " + str(measures['exec_mean']) + "s   variance: " + str(measures['exec_var'])
        print "Average tc difference is " + str(measures['tc_mean']) + " variance: " + str(measures['tc_var'])
        print "Average h difference is " + str(measures['h_mean']) + " variance: " + str(measures['h_var'])
        return measures
    '''
    Run a test without heuristic
    type: see below
    N: number of nodes
    iter: number of batch tests
    '''
    def test_noh(self, type, N, iter):
        return self.test_p(type,N,iter,2010)
    '''
    Run a test with the heuristic
    type: see below
    N: number of nodes
    iter: number of batch tests
    '''
    def test(self, type, N, iter):
	return self.test_p(type,N,iter,2009)
    '''
    Run a test without heuristic
    type: see below
    N: number of nodes
    iter: number of batch tests
    '''
    def test_noh_c(self, type, N, iter):
        return self.test_p(type,N,iter,2020)
    '''
    Run a test with the heuristic
    type: see below
    N: number of nodes
    iter: number of batch tests
    '''
    def test_c(self, type, N, iter):
	return self.test_p(type,N,iter,2019)


p = PrinceTestOONF()
# initialize the lists
measures_plaw_noh = []
measures_plaw = []
measures_plaw_noh_var = []
measures_plaw_var = []
measures_plaw_noh_c = []
measures_plaw_c = []
measures_plaw_noh_var_c = []
measures_plaw_var_c = []
x = []
jump = 50#20
max =1000 #500
min=300
iter=3
# run prince w & w/o heuristic
proc_noh = subprocess.Popen("exec ../../build/prince ../../input/test_noh.ini", shell=True)
proc = subprocess.Popen("exec ../../build/prince ../../input/test.ini", shell=True)

proc_noh_c = subprocess.Popen("exec ../../build/prince_c ../../input/test_noh_c.ini", shell=True, stdout=subprocess.PIPE)
proc_c = subprocess.Popen("exec ../../build/prince_c ../../input/test_c.ini", shell=True, stdout=subprocess.PIPE)
# cycle till the max values
try:
    for i in range(min, max+1,jump):
        size = i
        # run tests with and w/o heuristic
        t = p.test(1, size, iter)
        tnoh = p.test_noh(1, size, iter)
        t_c = p.test_c(1, size, iter)
        tnoh_c = p.test_noh_c(1, size, iter)
        # append the values in lists to plot them
        measures_plaw_noh.append(tnoh['exec_mean'])
        measures_plaw_noh_var.append(tnoh['exec_var'])
        measures_plaw.append(t['exec_mean'])
        measures_plaw_var.append(t['exec_var'])
        measures_plaw_noh_c.append(tnoh_c['exec_mean'])
        measures_plaw_noh_var_c.append(tnoh_c['exec_var'])
        measures_plaw_c.append(t_c['exec_mean'])
        measures_plaw_var_c.append(t_c['exec_var'])
        x.append(size)
except:
    print "Unexpected error:", sys.exc_info()
    exit(-1)
    #print("1====> ",proc_noh_c.stdout.read())
    #print("2====> ",proc_c.stdout.read())
    import sys
    kill(proc_noh_c.pid, signal.SIGTERM)
    kill(proc_c.pid, signal.SIGTERM)


print("qui")
from os import kill
import signal
kill(proc_noh.pid, signal.SIGTERM)
kill(proc.pid, signal.SIGTERM)
kill(proc_noh_c.pid, signal.SIGTERM)
kill(proc_c.pid, signal.SIGTERM)

# plot the values and the error bars with pyplot
plt.errorbar(x, measures_plaw_noh, yerr=measures_plaw_noh_var, label="C++ w/o h")
plt.errorbar(x, measures_plaw, yerr=measures_plaw_var, label="C++ w h")
plt.errorbar(x, measures_plaw_noh_c, yerr=measures_plaw_noh_var_c, label="C w/o h")
plt.errorbar(x, measures_plaw_c, yerr=measures_plaw_var_c, label="C w h")
plt.xlabel('size of graph (nodes)')
plt.ylabel('execution time (s)')
plt.legend(loc='upper center', shadow=True)
plt.axhline(1,color='k')
plt.savefig('res.png')
plt.show()

