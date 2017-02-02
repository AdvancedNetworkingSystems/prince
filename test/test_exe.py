#!/usr/bin/env python
import os,json,time,math,types
from subprocess import Popen, PIPE
import matplotlib.pyplot as plt
from numpy import var,mean
from graph_generator import Gen
from random import Random
import networkx as nx
r=Random(1234)


from heapq import heappush, heappop
from itertools import count
import networkx as nx
import random

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
    # rescaling
    #betweenness = _rescale(betweenness, len(G),normalized=normalized,directed=G.is_directed(),k=k)
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
    print s+"=> ",
    while Q:
        (dist, _, pred, v) = pop(Q)
        if v in D:
            continue  # already searched this node.
        print v,
        sigma[v] += sigma[pred]  # count paths
        S.append(v)
        D[v] = dist
        for w, edgedata in G[v].items():
            vw_dist = dist + edgedata.get(weight, 1)
            print "(",edgedata.get(weight, 1),")",
            if w not in D and (w not in seen or vw_dist < seen[w]):
                seen[w] = vw_dist
                #print "(",w,seen[w],vw_dist,")",
                push(Q, (vw_dist, next(c), v, w))
                sigma[w] = 0.0
                P[w] = [v]
            elif vw_dist == seen[w]:  # handle equal paths
                sigma[w] += sigma[v]
                P[w].append(v)
        print
    return S, P, sigma




def _accumulate_endpoints(betweenness, S, P, sigma, s):
    #print s+":",
    betweenness[s] += len(S) - 1
    delta = dict.fromkeys(S, 0)
    while S:
        w = S.pop()
        #print w,
        coeff = (1.0 + delta[w]) / sigma[w]
        for v in P[w]:
            delta[v] += sigma[v] * coeff
        if w != s:
            betweenness[w] += delta[w] + 1
    #print
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



def gen_graph(N):
    ge = Gen()
    ge.genGraph("PLAW", N)
    graph = ge.graph
    graph2=nx.Graph()
    for e in graph.edges():
        graph2.add_edge(e[0],e[1],weight=r.uniform(0,10))
    graph=graph2
    netjson = ge.composeNetJson(graph)
    json_netjson = json.dumps(netjson)
    text_file = open("input.json", "w+")
    text_file.write(json_netjson)
    text_file.close()
    return graph

def time_exe(is_c,heu):
    heu=str(heu)
    DEVNULL = open(os.devnull, 'wb', 0)
    exe="./c++.out "
    if is_c:
        exe="./c.out"
        heu=" "+heu+" 0"
    start = time.time()
    p = Popen(exe+heu,shell=True,stdout=PIPE,stderr=PIPE)
    #os.wait4(p.pid, 0)
    out, err = p.communicate()
    elapsed = time.time() - start
    title=""
    if is_c:
        title+="C "
    else:
        title+="C++ "
    if heu:
        title+=" with heu"
    else:
        title+=" without heu"
    if out:
        out=eval(out)
        global rounding
        #out= {k:round(v,rounding) for k,v in out.iteritems()}
        out= {k:v for k,v in out.iteritems()}
    return elapsed,out
"""
G=nx.Graph()
G.add_edge("0","3",weight=9.66453535692)
G.add_edge("1","2",weight=4.40732599175)
G.add_edge("1","3",weight=0.0749147005859)
G.add_edge("2","3",weight=9.10975962449)
print(betweenness_centrality(G,endpoints=True,weight='weight'))
import sys
sys.exit(0)
"""

rounding=20
start,end,jump=200,1600+1,100 
repetitions=5
max=int(math.ceil(float(end-start)/jump))*repetitions

res={}
res["x"]=[]
res["c_var"]=[]
res["c_mean"]=[]
res["c_eu_var"]=[]
res["c_eu_mean"]=[]
res["c++_var"]=[]
res["c++_mean"]=[]
res["c++_eu_var"]=[]
res["c++_eu_mean"]=[]
index=1
for i in range(start,end,jump):
    g=gen_graph(i)
    c_eu=[]
    c=[]
    cpp_eu=[]
    cpp=[]
    for j in xrange(repetitions):
        print(str(round(100*float(index)/max,2))+"%")
        index+=1
        timer,val1=time_exe(1,0)
        c.append(timer)
        timer,val2=time_exe(1,1)
        c_eu.append(timer)
        timer,val3=time_exe(0,0)
        cpp.append(timer)
        timer,val4=time_exe(0,1)
        cpp_eu.append(timer)
        #print(val1)
        actual_res=nx.betweenness_centrality(g,endpoints=True,weight='weight')
        #actual_res= {k:round(v,rounding) for k,v in actual_res.iteritems()}
        actual_res= {k:v for k,v in actual_res.iteritems()}
        #print(actual_res)
        if (not (actual_res==val1 and val1==val2)):
            print(nx.is_connected(g))
            print({k:(v-val1[k],v,val1[k]) for k,v in actual_res.iteritems() if v-val1[k]!=0})
            print({k:(v-val2[k],v,val2[k]) for k,v in actual_res.iteritems() if v-val2[k]!=0})
            #print("actual_res",actual_res)
            #print("1",val1)
            #print("2",val2)
            #print("3",val3)
            #print("4",val4)
            for e in g.edges(data='weight'):
                print('add_edge_graph(&g1,"'+str(e[0])+'","'+str(e[1])+'",'+str(e[2])+',0);')
            import sys
            sys.exit(0)
    res["c_var"].append(var(c))
    res["c_mean"].append(mean(c))
    res["c_eu_var"].append(var(c_eu))
    res["c_eu_mean"].append(mean(c_eu))
    res["c++_var"].append(var(cpp))
    res["c++_mean"].append(mean(cpp))
    res["c++_eu_var"].append(var(cpp_eu))
    res["c++_eu_mean"].append(mean(cpp_eu))
    res["x"].append(i)

plt.errorbar(res["x"], res["c++_mean"], yerr=res["c++_var"], label="C++ w/o h")
plt.errorbar(res["x"], res["c++_eu_mean"], yerr=res["c++_eu_var"], label="C++ w h")
plt.errorbar(res["x"], res["c_mean"], yerr=res["c_var"], label="C w/o h")
plt.errorbar(res["x"], res["c_eu_mean"], yerr=res["c_eu_var"], label="C w h")
plt.xlabel('size of graph (nodes)')
plt.ylabel('execution time (s)')
plt.legend(loc='upper center', shadow=True)
plt.axhline(1,color='k')
plt.savefig('res.png')
for var in (res["c_mean"], res["c_eu_mean"]):#,res["c++_mean"],res["c++_eu_mean"]):
    plt.annotate('%0.2f' % var[-1], xy=(1, var[-1]), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')
plt.show()
