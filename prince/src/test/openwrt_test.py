from graph_lib.graph_generator import Gen
from random import Random
from collections import OrderedDict
import time,re,os,json,datetime,networkx as nx
from numpy import var,mean
import socket


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


class server:
    def __init__(self,port=8080
                 ):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.bind(('', port))
        self.s.listen(10)
    def get_timer(self,graph):
        end=False
        p = re.compile(r"\d*\.\d+")
        exec_time=0
        while not end:
            #wait to accept a connection - blocking call
            conn, addr = self.s.accept()
            data=conn.recv(1024)
            if data.strip() == "/netjsoninfo filter graph ipv6_0/quit":
                json_netjson = json.dumps(composeNetJson(graph))
                conn.send(json_netjson)
                conn.close()
            elif data:
                toks = p.findall(data)
                if toks:
                    exec_time = float(toks[2])
                    conn.close()
                    end=True
        return exec_time
    def __exit__(self):
         self.s.close()


def main():
    s=server()
    for i in range(2,21):
        file="data/"+str(i*100)
        mkdir(file)
        executions= []
        for j in range(10):
            #print(str(round(float(((i-2)*10+j+1))/1.9,2))+"%")
            g=nx.read_weighted_edgelist(file+"/"+str(j))
            executions.append(s.get_timer(g))
        print(i*100,mean(executions),var(executions))
    s.__exit__()

if __name__ == "__main__":
    #print(datetime.datetime.now())
    #time.sleep(20)
    print(datetime.datetime.now())
    main()
    print(datetime.datetime.now())



#time.sleep(5)