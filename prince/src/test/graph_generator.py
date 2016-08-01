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
meshGraph = "MESH" # grid with diagonals
regularGraph = "REGULAR"
plainGrid = "TEST"
powerLaw = "PLAW"
allowedGraphs = [linearGraph, unitDisk, gridGraph, regularGraph,\
        plainGrid, ringGraph, powerLaw, meshGraph]


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
        Netjson['router_id'] = graph.nodes()[1]
        Netjson['nodes'] = []
        for node in graph.nodes():
            n = {}
            n['id']=str(node)
            Netjson['nodes'].append(n)

        Netjson['links'] = []
        for link in graph.edges():
            e = {}
            e['source'] = str(link[0])
            e['target'] = str(link[1])
            if graph.get_edge_data(e['source'], e['target']) is not None:
                e['cost'] = graph.get_edge_data(e['source'], e['target'])['weight']
            else:
                e['cost'] =1.0
            Netjson['links'].append(e)
        return Netjson

    def genCNGraph(self, N, T, E, S):
        g = CNGenerator(N=N, T=T, E=E, S=S)
        r = g.gen_core_network()
        g.attach_leave_nodes()
        for node in g.graph.nodes():
            if g.graph.has_edge(node, node):
                print "ERROR, node ", node, "has self loop"
                exit(1)
        self.graph= g.graph

    def genMeshGraph(self, N, seed):
        g = MeshGenerator(N, seed=seed)
        g.place_nodes()
        self.graph = g.graph

    def genGraph(self, graphKind, numNodes):
        """ Parameters
        --------------
        graphKind: string
        numNodes: integer

        """
        G=nx.Graph()
        print "Loading Graph"
        if graphKind == linearGraph:
            G.add_nodes_from(range(0,numNodes))
            for i in range(0,numNodes-1):
                G.add_edge(i,i+1)
        elif graphKind == ringGraph:
            G.add_nodes_from(range(0,numNodes))
            for i in range(0,numNodes):
                G.add_edge(i,(i+1)%numNodes)
        elif graphKind == unitDisk:
            r = 20
            # 90 nodes with 400*400 is ok, try to keep the same density
            #density = 90.0/(400*400)
            #area = numNodes/density
            #xSize = np.sqrt(area)

            xSize = 150 # use this to keep area fixed
            w = dict((i,r/2) for i in range(numNodes))
            for i in range(1000):
                random.jumpahead(i)
                p = dict((i,(random.uniform(0,xSize),random.uniform(0,xSize)))
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
            G= nx.random_regular_graph(degree,numNodes)
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
                positions[i] = (distance*(i%side), distance*(i/side))
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
            gamma=2
            powerlaw_gamma = lambda x: nx.utils.powerlaw_sequence(x, exponent=gamma)
            loop = True
            for i in range(1000):
                z = nx.utils.create_degree_sequence(numNodes,
                        powerlaw_gamma, max_tries=5000)
                G = nx.configuration_model(z)
                G = nx.Graph(G)
                G.remove_edges_from(G.selfloop_edges())
                mainC = nx.connected_component_subgraphs(G).next()
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

        print >> sys.stderr, "ok"
        print G
        self.graph = G
