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




rounding=8
#start,end,jump=100,1800+1,100
start,end,jump=200,2000+1,100
repetitions=10
max=int(math.ceil(float(end-start)/jump))*repetitions
"""
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
        actual_res= {int(k):round(v,rounding) for k,v in actual_res.iteritems()}
        #actual_res= {k:v for k,v in actual_res.iteritems()}
        #print(actual_res)
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
"""
res={'c_var': [8.0531150592833009e-06, 4.4061248163416171e-05, 0.00034514661733965108, 0.0016855954556291408, 0.0024220054017638402, 0.0084963717585020502, 0.020803703386837357, 0.011983563588003111, 0.079669045457720244, 0.028801808479386747, 0.069153552160341858, 0.07669758121234338, 0.092625837919240436, 0.30345302952495162, 0.28700727841626078, 0.54107609569897241, 1.2230355191677524, 1.3430094970811501, 3.1975211198702942], 'c_mean': [0.027500200271606445, 0.060748481750488283, 0.13979759216308593, 0.26595079898834229, 0.46777312755584716, 0.77427194118499754, 1.2226454973220826, 1.1672867774963378, 1.7314850091934204, 2.3704993009567259, 3.0334485054016112, 3.7171601057052612, 4.7671689987182617, 5.8932596921920775, 6.9988148689270018, 8.5919862270355232, 13.777107405662537, 16.738939094543458, 19.997123885154725], 'c++_var': [3.9117031865885105e-05, 0.00022745337186961476, 0.00014083390215148485, 0.001118368036059678, 0.0010415652170536304, 0.0037220557890015014, 0.0090168817806130612, 0.0049846140515347765, 0.032512697822287467, 0.017191783259414138, 0.0042760218030139189, 0.011577057211127909, 0.044705943337555711, 0.024043606192392417, 0.0260160426136531, 0.019422064477617483, 0.36447034526252364, 0.40623413805590253, 0.15801965034702389], 'c++_eu_var': [9.4925962827119289e-06, 5.8963161905012388e-06, 1.5906397635490065e-06, 1.4279000085934969e-05, 7.7767432253494926e-06, 1.9005490844961063e-05, 9.4057283723714151e-06, 2.1938176805633701e-05, 2.2188418811310839e-05, 4.0274595590403806e-05, 0.00014797285610029576, 1.4497478307475831e-05, 0.00015179978010110062, 5.5877660309420213e-05, 2.7582316084249216e-05, 5.8122232433674979e-05, 0.00026205195686088698, 0.00029550768380488536, 7.8799771143280839e-05], 'c++_eu_mean': [0.05405724048614502, 0.076056718826293945, 0.10157907009124756, 0.12499628067016602, 0.14882836341857911, 0.17801969051361083, 0.20513288974761962, 0.23350563049316406, 0.26050548553466796, 0.29218986034393313, 0.32280740737915037, 0.3647900104522705, 0.3999666213989258, 0.4353229761123657, 0.47389509677886965, 0.51137044429779055, 0.55000734329223633, 0.603157639503479, 0.65192382335662846], 'x': [200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000], 'c_eu_var': [7.5031237338407655e-06, 4.2981967345667735e-06, 7.1383455656359717e-06, 1.7973011431990925e-05, 1.3832606109644985e-05, 2.0984053971915273e-05, 1.8285333044900654e-05, 1.0670786266473441e-05, 4.5853396210304699e-06, 1.1527606307595306e-05, 6.944422384549399e-06, 2.2101235642821845e-05, 5.2012278069923911e-06, 4.8192351465559109e-06, 6.4988379472197272e-06, 2.6716035927165648e-06, 1.7066903412796823e-05, 1.3340732778601704e-05, 1.4588015688445921e-05], 'c++_mean': [0.44883415699005125, 1.0385329246520996, 1.9072376966476441, 3.093569779396057, 4.4886901378631592, 6.2157940149307249, 8.3844069242477417, 10.75867326259613, 13.504276084899903, 16.598437333106993, 19.913032054901123, 23.475367951393128, 27.585027456283569, 31.758708810806276, 36.351163244247438, 41.406256890296937, 46.959874510765076, 52.982080626487729, 60.031354022026065], 'c_eu_mean': [0.013851976394653321, 0.016682076454162597, 0.020876502990722655, 0.029550766944885253, 0.029927778244018554, 0.036294317245483397, 0.042105984687805173, 0.046993637084960939, 0.053896641731262206, 0.059630322456359866, 0.063849401473999021, 0.071954822540283209, 0.080156898498535162, 0.086387419700622553, 0.096307897567749018, 0.10257587432861329, 0.12589678764343262, 0.13613169193267821, 0.1485224485397339]}

plt.errorbar(res["x"], res["c++_mean"], yerr=res["c++_var"], label="C++ w/o h")
plt.errorbar(res["x"], res["c++_eu_mean"], yerr=res["c++_eu_var"], label="C++ w h")
plt.errorbar(res["x"], res["c_mean"], yerr=res["c_var"], label="C w/o h")
plt.errorbar(res["x"], res["c_eu_mean"], yerr=res["c_eu_var"], label="C w h")
plt.xlabel('size of graph (nodes)')
plt.ylabel('execution time (s)')
plt.yscale('log')
plt.legend(loc='upper center', shadow=True)
#plt.axhline(1,color='k')
for var in (res["c_mean"], res["c_eu_mean"],res["c++_mean"],res["c++_eu_mean"]):
    plt.annotate('%0.2f' % var[-1], xy=(1, var[-1]), xytext=(8, 0),
                 xycoords=('axes fraction', 'data'), textcoords='offset points')
plt.savefig('res.png')
plt.show()
