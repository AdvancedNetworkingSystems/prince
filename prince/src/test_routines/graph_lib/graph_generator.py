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
import networkx as nx
from cn_generator import CNGenerator
from mesh_generator import MeshGenerator

import sys
import numpy as np
import random
from collections import OrderedDict

linearGraph = "LINEAR"
ringGraph = "RING"
unitDisk = "UNIT"
gridGraph = "GRID"
meshGraph = "MESH"  # grid with diagonals
regularGraph = "REGULAR"
plainGrid = "TEST"
powerLaw = "PLAW"
allowedGraphs = [linearGraph, unitDisk, gridGraph, regularGraph, plainGrid, ringGraph, powerLaw, meshGraph]


class Gen:
        def composeNetJson(self, graph):
                """ Parameters
                graph: nx graph object
                """
                Netjson = OrderedDict()
                Netjson['type'] = 'NetworkGraph'
                Netjson['protocol'] = 'olsrv2'
                Netjson['version'] = 'poprouting custom'
                Netjson['revision'] = '0.11.3'
                Netjson['metric'] = 'ff_dat_metric'
                node = int(random.random()*1000) % len(graph.nodes())
                Netjson['router_id'] = graph.nodes()[node]
                Netjson['nodes'] = []
                for node in graph.nodes():
                        n = {}
                        n['id'] = str(node)
                        Netjson['nodes'].append(n)

                Netjson['links'] = []
                for link in graph.edges(data=True):
                        e = {}
                        e['source'] = str(link[0])
                        e['target'] = str(link[1])
                        if graph.get_edge_data(link[0], link[1]):
                                e['cost'] = graph.get_edge_data(link[0], link[1])['weight']
                        else:
                                e['cost'] = 1.0
                        Netjson['links'].append(e)
                return Netjson

        def genCNGraph(self, N, seed=None):
                g = CNGenerator(N=N,seed=seed)
                r = g.gen_core_network()
                g.attach_leave_nodes()
                for node in g.graph.nodes():
                        if g.graph.has_edge(node, node):
                                print "ERROR, node ", node, "has self loop"
                                exit(1)
                self.graph = g.graph

        def genMeshGraph(self, N, seed):
                g = MeshGenerator(N=N, seed=seed)
                g.place_nodes()
                self.graph = g.graph

        def genGraph(self, graphKind, numNodes):
                """ Parameters
                --------------
                graphKind: string
                numNodes: integer

                """
                G = nx.Graph()
                if graphKind == linearGraph:
                        G.add_nodes_from(range(0, numNodes))
                        for i in range(0, numNodes-1):
                                G.add_edge(i, i+1)
                elif graphKind == ringGraph:
                        G.add_nodes_from(range(0, numNodes))
                        for i in range(0, numNodes):
                                G.add_edge(i, (i+1)%numNodes)
                elif graphKind == unitDisk:
                        r = 20
                        # 90 nodes with 400*400 is ok, try to keep the same density
                        # density = 90.0/(400*400)
                        # area = numNodes/density
                        # xSize = np.sqrt(area)

                        xSize = 150  # use this to keep area fixed
                        w = dict((i,r/2) for i in range(numNodes))
                        for i in range(1000):
                                random.jumpahead(i)
                                p = dict((i, (random.uniform(0, xSize), random.uniform(0, xSize)))
                                                for i in range(numNodes))
                                G = nx.geographical_threshold_graph(numNodes,theta=1,alpha=1,
                                                pos=p, weight=w)
                                if nx.is_connected(G):
                                        break
                        if not nx.is_connected(G):
                                print >> sys.stderr, "Could not find a connected graph \
                                                with the given features in 1000 attempts!"
                                sys.exit(1)
                elif graphKind == regularGraph:
                        degree = 6
                        G = nx.random_regular_graph(degree,numNodes)
                elif graphKind == gridGraph or graphKind == meshGraph:
                        if graphKind == meshGraph:
                                radius = 90.0 # with diagonals
                        else:
                                radius = 70.0  # without diagonals
                        side = int(np.sqrt(numNodes))
                        if side*side != numNodes:
                                print >> sys.stderr, "Error, you want a squared \
                                                grid with",numNodes,"nodes, it's not a square"
                                sys.exit(1)
                        distance = 60
                        positions = {}
                        w = {}
                        for i in range(numNodes):
                                positions[i] = (distance*(i % side), distance*(i/side))
                                w[i] = radius/2
                        G = nx.geographical_threshold_graph(numNodes, theta=1,
                                        alpha=1, pos=positions, weight=w)
                        if not nx.is_connected(G):
                                print >> sys.stderr, "Error, something is \
                                                wrong with the graph definition"
                                sys.exit(1)
                elif graphKind == plainGrid:
                        side = int(np.sqrt(float(numNodes)))
                        G = nx.grid_2d_graph(side, side)
                        numNodes = side*side
                elif graphKind == powerLaw:
                        gamma = 2
                        powerlaw_gamma = lambda x: nx.utils.powerlaw_sequence(x, exponent=gamma)
                        loop = True
                        for i in range(1000):
                                z = nx.utils.create_degree_sequence(numNodes,
                                                powerlaw_gamma, max_tries=5000)
                                G = nx.configuration_model(z)
                                G = nx.Graph(G)
                                G.remove_edges_from(G.selfloop_edges())
                                mainC = list(nx.connected_component_subgraphs(G))[0]
                                if len(mainC.nodes()) >= numNodes*0.9:
                                        loop = False
                                        break
                        if loop:
                                print "ERROR: generating powerLow graph with \
                                                impossible parameters"
                                sys.exit()
                else:
                        errMsg = "Unknown graph type " + graphKind
                        print >> sys.stderr, errMsg
                        sys.exit(1)


                self.graph = G
