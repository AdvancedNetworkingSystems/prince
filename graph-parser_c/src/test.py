from GraphParser import GraphParser
import networkx as nx
import sys

import json
import random
from collections import OrderedDict

def composeNetJson(graph):
        """ Parameters
        graph: nx graph object
        """
        Netjson = OrderedDict()
        Netjson['type'] = 'NetworkGraph'
        Netjson['protocol'] = 'olsrv2'
        Netjson['version'] = 'poprouting custom'
        Netjson['revision'] = '0.11.3'
        Netjson['metric'] = 'ff_dat_metric'
        node = random.sample(graph.nodes(), 1)[0]
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
            e['cost'] = link[2]['weight']
            Netjson['links'].append(e)
        return Netjson

#G = nx.read_graphml(sys.argv[1])
G = nx.Graph()
for i in [0, 5, 10]:
    G.add_edge(str(i + 0), str(i + 1), weight=1)
    G.add_edge(str(i + 0), str(i + 2), weight=1)
    G.add_edge(str(i + 2), str(i + 3), weight=1)
    G.add_edge(str(i + 1), str(i + 4), weight=1)
    G.add_edge(str(i + 3), str(i + 4), weight=1)


G.add_edge(str(4), str(5), weight=1)
G.add_edge(str(9), str(10), weight=1)

print (nx.betweenness_centrality(G, weight='weight', endpoints=True, normalized=False))
print(GraphParser(json.dumps(composeNetJson(G)), True, True, True))  # NJGraph, Weight, Heuristic, Penalization,
