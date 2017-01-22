from reportlab.graphics.widgetbase import Face
from setuptools.command.rotate import rotate

from numpy.matlib import rand

import psutil
import threading
from time import sleep
import json
import numpy as np
from ctypes import cdll,c_double,py_object
from graph_generator import Gen
import datetime as dt
import networkx as nx
import random
from heapq import heappush, heappop
from itertools import count
import networkx as nx
import random


class CPUThread(threading.Thread):
    def __init__(self):
        super(CPUThread, self).__init__()
        self.usage=[]
        self.active=True
    def run(self):
        while self.active:
            self.usage.append(psutil.cpu_percent(interval=1))
            sleep(1)


def run_fun(f,n,h,w):
    t=CPUThread()
    t.start()
    elapsed_time=fun(n,h,w)
    t.active=False;
    t.join()
    print("data",np.mean(t.usage),elapsed_time)


#@profile
def compute(lib):
    lib.compute();

def fun(node_num,h,w):
        ge = Gen()
        ge.genGraph("PLAW", node_num)
        g=ge.graph

        s=json.dumps(ge.composeNetJson(g))
        lib_n='./libtest_cpp.so'
        if h==1:
            lib_n='./libtest_c.so'
        lib = cdll.LoadLibrary(lib_n)
        lib.get_res.restype=py_object
        lib.init(w);
        lib.parse(s);
        n1=dt.datetime.now()
        compute(lib)
        elapsed_time = (dt.datetime.now()-n1).microseconds
        elapsed_time=elapsed_time
        res=lib.get_res()

        print(dict(map(lambda (k,v): (k, round(v,8)), nx.betweenness_centrality(g,endpoints=True).iteritems()))==dict(map(lambda (k,v): (k, round(v,8)), res.iteritems())))
        print(nx.betweenness_centrality(g,endpoints=True))
        print(res)
        lib.destroy();
        return elapsed_time


import sys
args=sys.argv[1:]
#nodenum=int(args[0])
#is_c=int(args[1])
#heu=int(args[2])
#run_fun(fun,nodenum,is_c,heu)

#run_fun(fun,nodenum,1,0)
#run_fun(fun,nodenum,1,1)


#new

nodenum=7



def betweenness_centrality(G, k=None, normalized=True, weight=None,
                           endpoints=False,
                           seed=None):

    betweenness = dict.fromkeys(G, 0.0)  # b[v]=0 for v in G
    if k is None:
        nodes = G
    else:
        random.seed(seed)
        nodes = random.sample(G.nodes(), k)
    for s in nodes:
        # single source shortest paths
        S, P, sigma = _single_source_dijkstra_path_basic(G, s, weight)
        # accumulation
        betweenness = _accumulate_endpoints(betweenness, S, P, sigma, s)
    betweenness = _rescale(betweenness, len(G),normalized=normalized,directed=G.is_directed(),k=k)
    return betweenness


def _single_source_dijkstra_path_basic(G, s, weight='weight'):
    # modified from Eppstein
    S = []
    P = {}
    for v in G:
        P[v] = []
    sigma = dict.fromkeys(G, 0.0)    # sigma[v]=0 for v in G
    D = {}
    sigma[s] = 1.0
    push = heappush
    pop = heappop
    seen = {s: 0}
    c = count()
    Q = []   # use Q as heap with (distance,node id) tuples
    push(Q, (0, next(c), s, s))
    while Q:
        (dist, _, pred, v) = pop(Q)
        if v in D:
            continue  # already searched this node.
        sigma[v] += sigma[pred]  # count paths
        S.append(v)
        D[v] = dist
        for w, edgedata in G[v].items():
            vw_dist = dist + edgedata.get(weight, 1)
            if w not in D and (w not in seen or vw_dist < seen[w]):
                seen[w] = vw_dist
                push(Q, (vw_dist, next(c), v, w))
                sigma[w] = 0.0
                P[w] = [v]
            elif vw_dist == seen[w]:  # handle equal paths
                sigma[w] += sigma[v]
                P[w].append(v)
    return S, P, sigma


def _accumulate_endpoints(betweenness, S, P, sigma, s):
    betweenness[s] += len(S) - 1
    #print(s,' len ', len(S),(S))
    delta = dict.fromkeys(S, 0)
    while S:
        w = S.pop()
        coeff = (1.0 + delta[w]) / sigma[w]
        #print("======================")
        #print ('coeff for'+str(s)+":"+str(coeff))
        for v in P[w]:
            delta[v] += sigma[v] * coeff
        if w != s:
            #print ('@> ret_val'+str(w)+":"+str(delta[w]))
            betweenness[w] += delta[w] + 1
    #print(s,betweenness[s])
    return betweenness


def _rescale(betweenness, n, normalized, directed=False, k=None):
    if normalized is True:
        if n <= 2:
            scale = None  # no normalization b=0 for all nodes
        else:
            scale = 1.0 / ((n - 1) * (n - 2))
    else:  # rescale by 2 for undirected graphs
        if not directed:
            scale = 1.0 / 2.0
        else:
            scale = None
    if scale is not None:
        if k is not None:
            scale = scale * n / k
        for v in betweenness:
            betweenness[v] *= scale
    return betweenness

def print_json_f(g):
    print("")
    print("")
    print("{")
    print('"type": "NetworkGraph", ')
    print('"protocol": "OLSR",  ')
    print('"version": "0.6.6.1",  ')
    print('"revision": "98166feda69d60a27f81aad865a9c5c0",  ')
    print('"metric": "ETX",  ')
    print('"nodes": [')
    for i,n in enumerate(g.nodes()):
        print '{"id": "'+str(n)+'"}',
        if i!=len(g.nodes())-1:
            print(",")
    print('],"links": [')

    for i,e in enumerate(g.edges(data='weight')):
        print '{"source": "'+str(e[0])+'","target": "'+str(e[1])+'","cost": '+str(e[2])+'}',
        if i!=len(g.edges())-1:
            print(",")
    print("]")
    print("}")
nodenum=30

import time
start_time = time.time()

i=0
r=random
while i <25:
    nodenum=r.randint(25,1500)
    print(str(i)+"--- %s seconds ---" % round((time.time() - start_time),2)+" with #nodes: "+str(nodenum))
    i+=1
    ge = Gen()
    ge.genGraph("PLAW", nodenum)
    g=ge.graph
    g2=nx.Graph()
    for e in g.edges():
        g2.add_edge(e[0],e[1],weight=r.uniform(0,10))
    g=g2

    #g=nx.Graph()
    #g.add_weighted_edges_from([(0,20,1),(0,13,1),(0,30,1),(1,50,1),(1,54,1),(1,23,1),(2,66,1),(3,66,1),(4,48,1),(4,40,1),(4,26,1),(4,13,1),(4,15,1),(5,39,1),(5,55,1),(5,47,1),(5,19,1),(5,23,1),(5,58,1),(5,31,1),(6,66,1),(7,45,1),(8,9,1),(8,13,1),(8,71,1),(9,66,1),(9,41,1),(9,13,1),(9,48,1),(9,23,1),(9,56,1),(9,57,1),(9,28,1),(10,66,1),(11,57,1),(11,60,1),(12,56,1),(12,31,1),(12,71,1),(12,67,1),(12,47,1),(13,14,1),(13,18,1),(13,19,1),(13,32,1),(13,41,1),(13,42,1),(13,43,1),(13,48,1),(13,50,1),(13,55,1),(13,57,1),(13,64,1),(13,66,1),(13,67,1),(13,70,1),(13,72,1),(13,75,1),(13,76,1),(15,66,1),(16,66,1),(16,68,1),(17,67,1),(18,48,1),(18,66,1),(21,48,1),(21,23,1),(22,57,1),(23,66,1),(23,70,1),(23,74,1),(23,43,1),(23,48,1),(23,50,1),(23,42,1),(23,41,1),(23,57,1),(23,58,1),(23,26,1),(23,61,1),(24,35,1),(25,34,1),(26,49,1),(26,66,1),(27,48,1),(27,31,1),(28,42,1),(29,48,1),(29,42,1),(30,63,1),(33,56,1),(33,74,1),(33,43,1),(33,66,1),(34,66,1),(34,41,1),(34,75,1),(34,48,1),(34,50,1),(34,57,1),(34,56,1),(35,41,1),(35,48,1),(35,78,1),(35,56,1),(36,41,1),(36,57,1),(37,66,1),(37,62,1),(38,57,1),(38,66,1),(39,48,1),(41,43,1),(41,48,1),(41,66,1),(43,66,1),(43,48,1),(43,54,1),(44,48,1),(44,52,1),(45,48,1),(46,48,1),(48,51,1),(48,53,1),(48,54,1),(48,61,1),(48,66,1),(48,67,1),(48,70,1),(48,72,1),(48,79,1),(53,66,1),(55,66,1),(56,66,1),(56,57,1),(57,73,1),(57,61,1),(59,66,1),(62,66,1),(64,66,1),(65,66,1),(65,79,1),(66,70,1),(66,72,1),(66,73,1),(66,76,1),(66,77,1),(66,79,1),(68,69,1),(70,71,1),(72,74,1)])

    #el=[(0, 17), (0, 12), (0, 13), (1, 3), (1, 11), (1, 15), (2, 17), (3, 18), (3, 11), (3, 12), (3, 10), (4, 6), (5, 17), (7, 18), (7, 10), (8, 17), (9, 15), (11, 18), (11, 17), (12, 17), (12, 18), (14, 17), (15, 17), (16, 17), (17, 18), (18, 19)]
    #g=nx.from_edgelist(el)
    #g=nx.Graph()
    #for i,e in enumerate(el):
    #    g.add_edge(e[0],e[1],weight=r.uniform(0,10))


    s=json.dumps(ge.composeNetJson(g))

    lib_n='./libtest_c.so'
    lib = cdll.LoadLibrary(lib_n)
    lib.get_res.restype=py_object
    bc_nx_st = time.time()
    lib.init(1);
    lib.parse(s);
    compute(lib)
    res=lib.get_res()
    res={x:res[str(x)] for x in g.nodes()}
    lib.destroy()
    bc_nx_ts=round((time.time() - bc_nx_st),4)
    bc_nx_st = time.time()
    bc_nx=nx.betweenness_centrality(g,endpoints=True, weight="weight")
    bc_nx_tt=round((time.time() - bc_nx_st),4)
    print("time for c "+str(bc_nx_ts)+" time for py "+str(bc_nx_tt)+" ratio: "+str(round(bc_nx_ts/bc_nx_tt,2)))
    b=(dict(map(lambda (k,v): (k, round(v,8)),bc_nx.iteritems()))==dict(map(lambda (k,v): (k,round(v,8)), res.iteritems())))
    if not b and nx.is_connected(g):
        print '\033[93m'+ "Warning"
        print i,nx.is_connected(g)
        #print list(nx.biconnected_components(g))
        print(g.edges())
        bc=betweenness_centrality(g,endpoints=True, weight="weight")
        #print("correct     ",[round(bc[x],1) for x in bc])
        print("correct     ",bc)
        #print("keys",g.nodes())
        #res=([round(res[str(x)],1) for x in g.nodes()])
        print("heuristic   ",res)
        import numpy as np
        print("errors",[bc[k]-res[k] for k in res if (bc[k]-res[k])!=0])
        #for e in g.edges(data='weight'):
        #    print('add_edge_graph(&g1,"'+str(e[0])+'","'+str(e[1])+'",'+str(e[2])+',0);')
        #print_json_f(g)
        sys.exit(-1)