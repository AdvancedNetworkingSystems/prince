from graph_lib.graph_generator import Gen
from random import Random
from collections import OrderedDict
import time,re,os,socket,json,networkx as nx
from numpy import var,mean

def mkdir(path):
    if not os.path.exists(path):
        os.makedirs(path)
def generate_graphs():
    mkdir("data")
    ge = Gen()
    index=0
    r=Random(1235)
    for i in range(2,16):
        file="data/"+str(i*100)
        mkdir(file)
        for j in range(10):
            ge.genCNGraph(i*100/25,seed=(70*index+1))
            graph = ge.graph
            graph2=nx.Graph()
            for e in graph.edges():
                graph2.add_edge(e[0],e[1],weight=r.uniform(0,10))
            graph=graph2
            nx.write_weighted_edgelist(graph,file+"/"+str(j))
            #import hashlib
            #print(hashlib.md5(str(graph.edges())).hexdigest())
            #ensures reproducibility (and uniqueness)
            index+=1
    return

def composeNetJson(graph):
    Netjson = OrderedDict()
    Netjson['type'] = 'NetworkGraph'
    Netjson['protocol'] = 'olsrv2'
    Netjson['version'] = 'poprouting custom'
    Netjson['revision'] = '0.11.3'
    Netjson['metric'] = 'ff_dat_metric'
    Netjson['router_id'] = '0'
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
        if graph.get_edge_data(link[0], link[1]):
            e['cost'] = graph.get_edge_data(link[0], link[1])['weight']
        else:
            e['cost'] = 1.0
        Netjson['links'].append(e)
    return Netjson

def test(graph,port=1234):
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    serversocket.bind(('localhost', port))
    serversocket.listen(5)
    p = re.compile(r"\d*\.\d+")
    executions = []
    end=False
    exec_time=0
    while not end:
        (cs, address) = serversocket.accept()
        data = cs.recv(1024).decode("utf-8")
        if data.strip() == "/netjsoninfo filter graph ipv6_0/quit":
            json_netjson = json.dumps(composeNetJson(graph))
            cs.send(json_netjson)
            cs.close()
            print("a")
        else:
            print("b")
            if data:
                print("c")
                toks = p.findall(data)
                if toks:
                    exec_time = float(toks[2])
                    cs.close()
                    end=True
    return exec_time


#generate_graphs()
def main():
    for i in range(2,16):
        file="data/"+str(i*100)
        mkdir(file)
        executions= []
        print(i)
        for j in range(10):
            print(j)
            g=nx.read_weighted_edgelist(file+"/"+str(j))
            executions.append(test(g))
        print(mean(executions),var(executions))
        break

if __name__ == "__main__":
    main()



#time.sleep(5)