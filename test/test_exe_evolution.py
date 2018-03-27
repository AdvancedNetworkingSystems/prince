#!/usr/bin/env python
import json,time
from subprocess import Popen, PIPE
import matplotlib.pyplot as plt
from numpy import var,mean
from graph_lib.graph_generator import Gen
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
        graph2.add_edge(e[0],e[1],weight=r.uniform(0,10))
    graph=graph2
    write_graph_to_file(ge,graph)
    return graph

def time_exe(save,graph,prob):
    exe="./c.out 1 "
    start = time.time()
    s="0"
    if save:
        s="1"
    p = Popen(exe+s,shell=True,stdout=PIPE,stderr=PIPE)
    out, err = p.communicate()
    elapsed = time.time() - start
    rec="Recompute" in out
    actual=nx.betweenness_centrality(graph,endpoints=True,weight="weight")
    res=eval(out.replace("Recompute",""))
    #errors=([v-res[int(str(k))] for k,v in actual.iteritems() if (v-res[int(str(k))]!=0)])
    #errors=([v-actual[str(k)] for k,v in res.iteritems() if (v-actual[str(k)]!=0)])
    errors=([v-actual[str(k)] for k,v in res.iteritems()])
    err=0
    if mean(errors):
        err=(max(errors))/mean(actual.values())
        #print prob,mean(errors),mean(actual.values()),str((mean(errors)*100)/mean(actual.values()))+"%",str(err*100)+"%"
    return elapsed,rec or not save,err

"""
#do once
ge = Gen()
ge.genGraph("PLAW", 100)
graph2=nx.Graph()
graph = ge.graph
for e in graph.edges():
    graph2.add_edge(e[0],e[1],weight=r.uniform(0,10))
graph=graph2
nx.write_weighted_edgelist(graph,"base.graph");
sys.exit(0)
"""

def evolve_graph(g,change_probability):
    edges=g.edges()
    change=r.uniform(0,1)
    if change<change_probability:
        add=r.uniform(0,1)>.5
        if add:
            nodes=g.nodes()
            node_num=len(nodes)-1
            while True and len(edges)>2:
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



def run_evolution(ge,g,steps_number,prob,save):
    timing=[]
    errors=[]
    timer=0
    num_recompute=-1
    for i in xrange(steps_number):
        if i % 100 ==0 and i>0:
            print(i)
        write_graph_to_file(ge,g)
        evolve_graph(g,prob)
        t,b,err=time_exe(save,g,prob)
        if b:
            num_recompute+=1
        #timer=t
        timer+=t
        timing.append(timer)
        errors.append(err)
    return timing,num_recompute,errors

ge = Gen()
g=nx.read_weighted_edgelist("base.graph");

steps_number=1000
res={}
x=list(xrange(steps_number))
res["c_without"],res["c_without_r"],res["c_without_err"]=run_evolution(ge,g,steps_number,0,0)
print("a")
g=nx.read_weighted_edgelist("base.graph")
res["c_with_0.01"],res["c_with_0.01_r"],res["c_with_0.01_err"]=run_evolution(ge,g,steps_number,0.0,1)
print("b")
g=nx.read_weighted_edgelist("base.graph")
res["c_with_0.1"],res["c_with_0.1_r"],res["c_with_0.1_err"]=run_evolution(ge,g,steps_number,0.1,1)
print("c")
#g=nx.read_weighted_edgelist("base.graph")
#res["c_with_0.25"],res["c_with_0.25_r"]=run_evolution(ge,g,steps_number,0.25,1)
#print("d")
g=nx.read_weighted_edgelist("base.graph")
res["c_with_0.5"],res["c_with_0.5_r"],res["c_with_0.5_err"]=run_evolution(ge,g,steps_number,0.5,1)
print("e")
g=nx.read_weighted_edgelist("base.graph")
res["c_with_0.99"],res["c_with_0.99_r"],res["c_with_0.99_err"]=run_evolution(ge,g,steps_number,0.99,1)

plt.subplots_adjust(left=0.1, right=0.8, top=0.9, bottom=0.1)
plt.errorbar(x, res["c_without"],yerr=res["c_without_err"],  label="Recompute ")
plt.errorbar(x, res["c_with_0.01"],yerr=res["c_with_0.01_err"],  label="Test-recompute ($p_{change}$=0.01)")
plt.errorbar(x, res["c_with_0.1"],yerr=res["c_with_0.1_err"],  label="Test-recompute ($p_{change}$=0.1)")
#plt.errorbar(x, res["c_with_0.25"],  label="Test-recompute ($p_{change}$=0.25)")
plt.errorbar(x, res["c_with_0.5"],yerr=res["c_with_0.5_err"],  label="Test-recompute ($p_{change}$=0.5)")
plt.errorbar(x, res["c_with_0.99"],yerr=res["c_with_0.99_err"],  label="Test-recompute ($p_{change}$=0.99)")
plt.xlabel('Executions (-)')
plt.ylabel('total delay for recomputation (s)')
plt.ylabel('total delay for recomputation (s)',)
plt.legend(loc='upper center', shadow=True)
#plt.axhline(1,color='k')
plt.savefig('res.png')
#for var in ("c_without","c_with_0.01","c_with_0.1","c_with_0.5","c_with_0.99"):#",c_with_0","c_with_0.25"):

series = res["c_without"]
text= str(int(round(series[-1],2)))+"("+str(res["c_without_r"])+")"
plt.annotate(text, xy=(1, series[-1]), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')

series = res["c_with_0.01"]
text= str(int(round(series[-1],2)))+"("+str(res["c_with_0.01_r"])+")"
plt.annotate(text, xy=(1, series[-1]*0.97), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')
series = res["c_with_0.1"]
text= str(int(round(series[-1],2)))+"("+str(res["c_with_0.1_r"])+")"
plt.annotate(text, xy=(1, series[-1]*0.99), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')
series = res["c_with_0.5"]
text= str(int(round(series[-1],2)))+"("+str(res["c_with_0.5_r"])+")"
plt.annotate(text, xy=(1, series[-1]*1.01), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')
series = res["c_with_0.99"]
text= str(int(round(series[-1],2)))+"("+str(res["c_with_0.99_r"])+")"
plt.annotate(text, xy=(1, series[-1]*1.03), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')

ax=plt.twinx()
plt.ylabel(" Time spent (steps_number of recomputations)")
ax.yaxis.set_label_coords(1.2, 0.6)
plt.setp( ax.get_yticklabels(), visible=False)
plt.setp( ax.get_yticklines(), visible=False)


plt.savefig('res_exe.png')
plt.show()
