#!/usr/bin/env python
import os,json,time,math,types
from subprocess import Popen, PIPE
import matplotlib.pyplot as plt
from numpy import var,mean
from graph_lib.graph_generator import Gen
from random import Random
import networkx as nx
r=Random(1234)


def gen_graph(N):
        ge = Gen()
        ge.genGraph("PLAW", N)
        graph = ge.graph
        graph2=nx.Graph()
        for e in graph.edges():
                graph2.add_edge(e[0],e[1],weight=r.uniform(0,10))
        graph=graph2
        netjson = ge.composeNetJson(graph)
        #print(netjson)
        json_netjson = json.dumps(netjson)
        text_file = open("input.json", "w+")
        text_file.write(json_netjson)
        text_file.close()
        return graph

def time_exe(is_c,heu):
        heu=str(heu)
        exe="./c++.out "
        if is_c:
                exe="./c.out"
                heu=" "+heu+" 0"
        start = time.time()
        #print(exe+heu)
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
                #print(out)
                out=eval(out)
                global rounding
                out= {k:round(v,rounding) for k,v in out.iteritems()}
                #out= {k:v for k,v in out.iteritems()}
        return elapsed,out



rounding=2
#start,end,jump=100,1800+1,100
start,end,jump=200,2000+1,100
repetitions=10
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
        #g=nx.read_weighted_edgelist()
        c_eu=[]
        c=[]
        cpp_eu=[]
        cpp=[]
        for j in xrange(repetitions):
                g=nx.read_weighted_edgelist("data/"+str(i)+"/"+str(j))
                netjson = Gen().composeNetJson(g)
                #print(netjson)
                json_netjson = json.dumps(netjson)
                text_file = open("input.json", "w+")
                text_file.write(json_netjson)
                text_file.close()

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
                actual_res= {k:round(v,rounding) for k,v in actual_res.iteritems()}
                #actual_res= {k:v for k,v in actual_res.iteritems()}
                #print(actual_res)
                continue
                if (not (actual_res==val1 and val1==val2)):
                        #print({k:(v-val1[k],v,val1[k]) for k,v in actual_res.iteritems() if v-val1[k]!=0})
                        #print({k:(v-val2[k],v,val2[k]) for k,v in actual_res.iteritems() if v-val2[k]!=0})
                        print("val1",val1)
                        print("val2",val2)
                        print("actual_res",actual_res)
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
print(res)
exit(0)
plt.errorbar(res["x"], res["c++_mean"], yerr=res["c++_var"], label="C++ w/o h")
plt.errorbar(res["x"], res["c++_eu_mean"], yerr=res["c++_eu_var"], label="C++ w h")
plt.errorbar(res["x"], res["c_mean"], yerr=res["c_var"], label="C w/o h")
plt.errorbar(res["x"], res["c_eu_mean"], yerr=res["c_eu_var"], label="C w h")
plt.xlabel('size of graph (nodes)')
plt.ylabel('execution time (s)')
plt.legend(loc='upper center', shadow=True)
#plt.axhline(1,color='k')
for var in (res["c_mean"], res["c_eu_mean"],res["c++_mean"],res["c++_eu_mean"]):
        plt.annotate('%0.2f' % var[-1], xy=(1, var[-1]), xytext=(8, 0),
                                 xycoords=('axes fraction', 'data'), textcoords='offset points')
plt.savefig('res.png')
plt.show()
