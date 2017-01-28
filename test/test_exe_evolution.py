#!/usr/bin/env python
import os,json,time,math,types
from subprocess import Popen, PIPE
import matplotlib.pyplot as plt
from numpy import var,mean
from graph_generator import Gen
from random import Random
import networkx as nx
r=Random(1234)

def write_graph_to_file(ge,g):
    netjson = ge.composeNetJson(g)
    json_netjson = json.dumps(netjson)
    text_file = open("input.json", "w+")
    text_file.write(json_netjson)
    text_file.close()

def gen_graph(N):
    ge = Gen()
    ge.genGraph("PLAW", N)
    graph = ge.graph
    graph2=nx.Graph()
    for e in graph.edges():
        graph2.add_edge(e[0],e[1],weight=1)#r.uniform(0,10))
    graph=graph2
    write_graph_to_file(ge,graph)
    return graph

def time_exe(save):
    exe="./c.out 1 "
    start = time.time()
    s="0"
    if save:
        s="1"
    p = Popen(exe+s,shell=True,stdout=PIPE,stderr=PIPE)
    out, err = p.communicate()
    elapsed = time.time() - start
    return elapsed,"Recompute" in out or not save

"""
#do once
ge = Gen()
ge.genGraph("PLAW", 1000)
graph2=nx.Graph()
graph = ge.graph
for e in graph.edges():
    graph2.add_edge(e[0],e[1],weight=r.uniform(0,10))
graph=graph2
nx.write_weighted_edgelist(graph,"base.graph");
"""

def evolve_graph(g,change_probability):
    edges=g.edges()
    change=r.uniform(0,1)
    if change<change_probability:
        add=r.uniform(0,1)>.5
        if add:
            nodes=g.nodes()
            node_num=len(nodes)-1
            while True:
                n1_id=r.randint(0,node_num)
                n1=nodes[n1_id]
                n2_id=n1_id
                while n2_id==n1_id:
                    n2_id=r.randint(0,node_num)
                n2=nodes[n2_id]
                if ((n1,n2) in edges):
                    continue
                g.add_weighted_edges_from([(n1,n2,r.uniform(0,10))])
                break;
        else:
            e=edges[r.randint(0,len(edges)-1)]
            g.remove_edge(e[0],e[1])



def run_evolution(ge,g,number,prob,save):
    timing=[]
    timer=0
    num_recompute=0
    for i in xrange(number):
        if i % 100 ==0 and i>0:
            print(i)
        write_graph_to_file(ge,g)
        evolve_graph(g,prob)
        t,b=time_exe(save)
        if b:
            num_recompute+=1
        timer+=t
        timing.append(timer)
    return timing,num_recompute

ge = Gen()
g=nx.read_weighted_edgelist("base.graph");

number=1000
res={}
x=list(xrange(number))
res["c_without"],res["c_without_r"]=run_evolution(ge,g,number,0,0)
print("a")
g=nx.read_weighted_edgelist("base.graph")
res["c_with_0.01"],res["c_with_0.01_r"]=run_evolution(ge,g,number,0.0,1)
print("b")
g=nx.read_weighted_edgelist("base.graph")
res["c_with_0.1"],res["c_with_0.1_r"]=run_evolution(ge,g,number,0.1,1)
print("c")
#g=nx.read_weighted_edgelist("base.graph")
#res["c_with_0.25"],res["c_with_0.25_r"]=run_evolution(ge,g,number,0.25,1)
#print("d")
g=nx.read_weighted_edgelist("base.graph")
res["c_with_0.5"],res["c_with_0.5_r"]=run_evolution(ge,g,number,0.5,1)
print("e")
g=nx.read_weighted_edgelist("base.graph")
res["c_with_0.99"],res["c_with_0.99_r"]=run_evolution(ge,g,number,0.99,1)

plt.errorbar(x, res["c_without"],  label="Recompute ")
plt.errorbar(x, res["c_with_0.01"],  label="Test-recompute ($p_{change}$=0.01)")
plt.errorbar(x, res["c_with_0.1"],  label="Test-recompute ($p_{change}$=0.1)")
#plt.errorbar(x, res["c_with_0.25"],  label="Test-recompute ($p_{change}$=0.25)")
plt.errorbar(x, res["c_with_0.5"],  label="Test-recompute ($p_{change}$=0.5)")
plt.errorbar(x, res["c_with_0.99"],  label="Test-recompute ($p_{change}$=0.99)")
plt.xlabel('Executions (-)')
plt.ylabel('total delay for recomputation (s)')
plt.ylabel('total delay for recomputation (s)',)
plt.legend(loc='upper center', shadow=True)
#plt.axhline(1,color='k')
plt.savefig('res.png')
#for var in ("c_without","c_with_0.01","c_with_0.1","c_with_0.5","c_with_0.99"):#",c_with_0","c_with_0.25"):

series = res["c_without"]
text= res["c_without_r"]
plt.annotate(text, xy=(1, series[-1]), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')

series = res["c_with_0.01"]
text= res["c_with_0.01_r"]
plt.annotate(text, xy=(1, series[-1]*0.97), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')
series = res["c_with_0.1"]
text= res["c_with_0.1_r"]
plt.annotate(text, xy=(1, series[-1]*0.99), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')
series = res["c_with_0.5"]
text= res["c_with_0.5_r"]
plt.annotate(text, xy=(1, series[-1]*1.01), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')
series = res["c_with_0.99"]
text= res["c_with_0.99_r"]
plt.annotate(text, xy=(1, series[-1]*1.03), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')

ax=plt.twinx()
plt.ylabel("Number of computations (-)")
ax.yaxis.set_label_coords(1.09, 0.6)
plt.setp( ax.get_yticklabels(), visible=False)


plt.savefig('res_exe.png')
plt.show()
