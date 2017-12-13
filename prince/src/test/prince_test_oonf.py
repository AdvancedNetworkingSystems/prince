import re
import socket
import json
import numpy as np
import Queue
import time
from poprouting import ComputeTheoreticalValues
from graph_generator import Gen
import threading


class PrinceTestOONF:
        '''
        Generate a topology in netjson format and send it to a client
        type: type of topology generated: 0 CN, 1 PLAW, 2 MESH
        cs: clientsocket object
        N: nubmer of nodes of the topology
        '''

        def __init__(self):
                self.workers_order_q = Queue.Queue()
                self.workers_result_q = Queue.Queue()
                self.workers = []
                self.go = 1
                for i in range(1, 2):
                        t = threading.Thread(target=self.generatorWorker, args=[i])
                        self.workers.append(t)
                        t.daemon = True
                        t.start()

        def terminate_workers(self):
                self.go = 0

        def generatorWorker(self, worker_id):
                while(self.go):
                        # READ FROM SHARED FIFO QUEUE
                        order = self.workers_order_q.get()
                        print ("thread " + str(worker_id) + " got an order\n")
                        # Gen the graph
                        T = int(order['N'] * 1.2)
                        E = order['N'] * 1.8
                        ge = Gen()
                        if order['type'] == 0:
                                ge.genCNGraph(order['N'])
                        if order['type'] == 1:
                                ge.genGraph("PLAW", order['N'])
                        if order['type'] == 2:
                                ge.genMeshGraph(order['N'], 23)
                        if order['type'] == 3:
                                ge.genGraph("GRID", order['N'])
                        # Calculate the timers
                        netjson = Gen.composeNetJson(ge.graph)
                        print ("thread " + str(worker_id) + " calc timers with nx\n")
                        h_py, tc_py = self.calculateTimers(ge.graph, netjson['router_id'])
                        # WRITE TO SHARED FIFO QUEUE
                        result = (netjson, h_py, tc_py)
                        print ("thread " + str(worker_id) + " put result\n")
                        self.workers_result_q.put(result)
                        print "ORDER QSIZE " + str(self.workers_order_q.qsize())
                        print "RESULT QSIZE " + str(self.workers_result_q.qsize())

        '''
        Calculate timers using the poprouting function CalculateTheoreticalValues
        and choose the one that matches our "router_id"
        '''
        def calculateTimers(self, graph, router_id):
                # compute the timers values using the poprouting python lib
                ctv = ComputeTheoreticalValues(graph=graph)
                hello_py = tc_py = 0
                # search for the timer we are calculating (the 'router_id' one)
                for node in ctv.node_list:
                        if node == router_id:
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
                self.p = re.compile(r"\d*\.\d+")
                tc_cpp = 0
                self.executions = []
                self.hs = []
                self.tcs = []
                ge = Gen()
                # Order iter graphs of size 'N' and type to the workers
                for i in range(1, iter + 1):
                        order = {}
                        order['type'] = type
                        order['N'] = N
                        self.workers_order_q.put(order)
                self.workers_order_q.task_done()
                print("Order made")
                while iter:
                        # accept connections
                        (cs, address) = serversocket.accept()
                        data = cs.recv(1024).decode("utf-8")
                        # simulate the oonf's netjson command to get the topology
                        if data.strip() == "/netjsoninfo filter graph ipv6_0/quit":
                                # Pop the generated graph from the workers queue, push topology to prince and calculate timers(python)
                                result = self.workers_result_q.get()
                                self.netjson = result[0]
                                self.h_py = result[1]
                                self.tc_py = result[2]
                                cs.send(json.dumps(self.netjson))
                                cs.close()

                        else:
                                # search for the timers' values using the regex
                                if(not self.parse_data(data)):
                                        print json.dumps(self.netjson)
                                        print "\n\n\n"
                                cs.close()
                                iter -= 1

                executions = np.array(self.executions)
                hs = np.array(self.hs)
                tcs = np.array(self.tcs)
                mat = np.dstack((executions, hs, tcs)).squeeze()
                return mat

        def parse_data(self, data):
                if data:
                        toks = self.p.findall(data)
                        if toks:
                                try:
                                        tc_cpp = float(toks[0])
                                        hello_cpp = float(toks[1])
                                        exec_time = 0#float(toks[2])
                                except IndexError:
                                        print ("ERRORE!!!\n\n\n\n")
                                        print data
                                        return False
                                # calculate the percentage error between the reference (py) and the measured (C[++]) values
                                tc_err = abs(tc_cpp - self.tc_py) / ((tc_cpp + self.tc_py) / 2) * 100
                                h_err = abs(hello_cpp - self.h_py) / ((hello_cpp + self.h_py) / 2) * 100
                                self.hs.append(h_err)
                                self.tcs.append(tc_err)
                                self.executions.append(exec_time)
                                return True
