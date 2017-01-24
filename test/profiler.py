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



def betweenness_centrality(G, scale=False ,weight="weight"):

    betweenness = dict.fromkeys(G, 0.0)  # b[v]=0 for v in G
    nodes = G
    for s in nodes:
        # single source shortest paths
        S, P, sigma = _single_source_dijkstra_path_basic(G, s, weight)
        # accumulation
        betweenness = _accumulate_endpoints(betweenness, S, P, sigma, s)
    if scale:
        betweenness = _rescale(betweenness, len(G))
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


def _rescale(betweenness, n, normalized=True, directed=False, k=None):
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
nodenum=50

import time
start_time = time.time()

i=0
r=random

def biconnected_dfs(G, components=True):
    # depth-first search algorithm to generate articulation points
    # and biconnected components
    visited = set()
    a=set()
    for start in G:
        if start in visited:
            continue
        discovery = {start: 0}  # time of first discovery of node during search
        low = {start: 0}
        root_children = 0
        visited.add(start)
        edge_stack = []
        stack = [(start, start, iter(G[start]))]
        while stack:
            grandparent, parent, children = stack[-1]
            a.add(parent)
            try:
                child = next(children)
                if grandparent == child:
                    continue
                if child in visited:
                    if discovery[child] <= discovery[parent]:  # back edge
                        low[parent] = min(low[parent], discovery[child])
                        if components:
                            edge_stack.append((parent, child))
                else:
                    low[child] = discovery[child] = len(discovery)
                    visited.add(child)
                    stack.append((parent, child, iter(G[child])))
                    if components:
                        edge_stack.append((parent, child))
            except StopIteration:
                stack.pop()
                if len(stack) > 1:
                    if low[parent] >= discovery[grandparent]:
                        if components:
                            ind = edge_stack.index((grandparent, parent))
                            yield edge_stack[ind:]
                            edge_stack = edge_stack[:ind]
                        else:
                            yield grandparent
                    low[grandparent] = min(low[parent], low[grandparent])
                elif stack:  # length 1 so grandparent is root
                    root_children += 1
                    if components:
                        ind = edge_stack.index((grandparent, parent))
                        yield edge_stack[ind:]
        if not components:
            # root node is articulation point if it has more than 1 child
            if root_children > 1:
                yield start
        else:
            a=set()

#test on cliques for single connected components to test whether it is faster orig brandes
#or heuristic

ge = Gen()
g=nx.complete_graph(200)
s=json.dumps(ge.composeNetJson(g))
times=[]
for i in xrange(1):
    lib_n='./libtest_c.so'
    lib = cdll.LoadLibrary(lib_n)
    lib.get_res.restype=py_object
    bc_nx_st = time.time()
    lib.init(1);
    lib.parse(s);
    compute(lib)
    res=lib.get_res()
    lib.destroy()
    bc_nx_ts=(time.time() - bc_nx_st)
    times.append(bc_nx_ts)
    print(res)
    print(nx.betweenness_centrality(g,endpoints=True))
print("non orig",np.mean(times),np.var(times))
sys.exit(0)


errors=[]
while i <100:
    nodenum=r.randint(25,200)#1500)
    ge = Gen()
    ge.genGraph("PLAW", nodenum)
    g=ge.graph
    if nx.is_connected(g):
        continue
    print(str(i)+"--- %s seconds ---" % round((time.time() - start_time),2)+" with #nodes: "+str(nodenum))
    i+=1
    g2=nx.Graph()
    for e in g.edges():
        g2.add_edge(e[0],e[1],weight=r.uniform(0,10))
    g=g2

    s=json.dumps(ge.composeNetJson(g))
    lib_n='./libtest_c.so'
    lib = cdll.LoadLibrary(lib_n)
    lib.get_res.restype=py_object
    bc_nx_st = time.time()
    lib.init(1);
    lib.parse(s);
    compute(lib)
    res=lib.get_res()
    lib.destroy()
    bc_nx_ts=round((time.time() - bc_nx_st),4)
    res={x:res[str(x)] for x in g.nodes()}
    bc_nx_st = time.time()
    bc_nx=nx.betweenness_centrality(g,endpoints=True, weight="weight")
    bc_nx_tt=round((time.time() - bc_nx_st),4)
    print("time for c "+str(bc_nx_ts)+" time for py "+str(bc_nx_tt)+" ratio: "+str(round(bc_nx_ts/bc_nx_tt,2)))
    #b=(dict(map(lambda (k,v): (k, round(v,8)),bc_nx.iteritems()))==dict(map(lambda (k,v): (k,round(v,8)), res.iteritems())))
    b=bc_nx==res
    errors+=filter(lambda x:x!=0,[v-res[k] for k,v in bc_nx.iteritems()])
    if not b:
        #print '\033[91m'+ "Warning"
        #print '\033[90m'+str((i,nx.is_connected(g)))
        print(i,nx.is_connected(g),[len(u) for u in list(nx.connected_components(g))])
        #print list(nx.biconnected_components(g))
        print(g.edges())
        bc=nx.betweenness_centrality(g,endpoints=True, weight="weight")
        #print("correct     ",[round(bc[x],1) for x in bc])
        print("correct     ",bc)
        #print("keys",g.nodes())
        #res=([round(res[str(x)],1) for x in g.nodes()])
        print("heuristic   ",res)
        import numpy as np
        print("errors",[bc[k]-res[k] for k in res if (bc[k]-res[k])!=0])
        for e in g.edges(data='weight'):
            print('add_edge_graph(&g1,"'+str(e[0])+'","'+str(e[1])+'",'+str(e[2])+',0);')
        #print_json_f(g)
        sys.exit(-1)
print("l",len(errors))
print(errors)