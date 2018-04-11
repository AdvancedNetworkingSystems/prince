from subprocess import Popen,PIPE
import socket
from graph_generator import Gen
import networkx as nx
from json import dumps
from random import Random
from numpy import mean,var
from matplotlib.pyplot import errorbar,xlabel,ylabel,legend,axhline,savefig,show

prince="../prince/build/prince"

conf="../prince/input/test.ini"
conf_heu="../prince/input/test_h.ini"


prince_c="../prince/build/prince_c"

c_conf="../prince/input/test_c.ini"
c_conf_heu="../prince/input/test_h_c.ini"

r=Random(1234)

def test(exe_p,conf_p,port,repetitions=2,start=10,end=10,step=1):
    print(exe_p+" "+conf_p)
    Popen("exec "+exe_p+" "+conf_p,shell=True, stdout=PIPE)
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    serversocket.bind(('localhost', port))
    serversocket.listen(5)
    means=[]
    variances=[]
    indexes=[]
    for i in range(start,end+1,step):
        repeat=repetitions
        exec_times=[]
        print(i,repeat)
        while repeat:
            (cs, address) = serversocket.accept()
            data=cs.recv(1024).decode("utf-8")
            if data.strip() == "/netjsoninfo filter graph ipv6_0/quit":
                ge = Gen()
                ge.genGraph("PLAW", i)
                graph=ge.graph
                graph2=nx.Graph()
                for e in graph.edges():
                    graph2.add_edge(e[0],e[1],weight=r.uniform(0,10))
                graph=graph2
                cs.send(dumps(ge.composeNetJson(graph)))
            else:
                exec_times.append(float(data[data.rfind("=")+1:]))
                repeat-=1
            cs.close()
        indexes.append(i)
        print("exec_times",exec_times)
        variances.append(var(exec_times))
        means.append(mean(exec_times))
    return indexes, means, variances


#x,measures_plaw,measures_plaw_var = test(prince,conf,2009)
x,measures_plaw_h,measures_plaw_var_h = test(prince,conf_heu,2010)
print(x,measures_plaw_h,measures_plaw_var_h)
#print(measures_plaw,measures_plaw_h)

#x,measures_plaw_c,measures_plaw_var_c = test(prince_c,c_conf,2019)
#x,measures_plaw_h_c,measures_plaw_var_h_c = test(prince_c,c_conf_heu,2020)
#print(measures_plaw_c,measures_plaw_h_c)
"""

#errorbar(x, measures_plaw, yerr=measures_plaw_var, label="C++ w/o h")
#errorbar(x, measures_plaw_h, yerr=measures_plaw_var_h, label="C++ w h")
#errorbar(x, measures_plaw_c, yerr=measures_plaw_var_c, label="C w/o h")
#errorbar(x, measures_plaw_h_c, yerr=measures_plaw_var_h_c, label="C w h")
xlabel('size of graph (nodes)')
ylabel('execution time (s)')
legend(loc='upper center', shadow=True)
#axhline(1,color='k')
#savefig('res.png')
show()
"""