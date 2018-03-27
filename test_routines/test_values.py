#data comes from ninux, FFGraz, FFWien networks, artifical network derived adding node failures

import networkx as nx
#https://ans.disi.unitn.it/redmine/projects/pop-routing/repository?utf8=%E2%9C%93&rev=TON_paper
from compute_theoretical_values import  ComputeTheoreticalValues
from glob import glob
from numpy import std,mean
from random import randint,random

def generate_failure_network():
    g= nx.read_weighted_edgelist('edgelist/ninux/0', nodetype=int)
    index=0
    for i in range(10):
        random.seed(1234)
        bc=nx.betweenness_centrality(g)
        max_node=(max(bc, key=bc.get))
        nodes=[(k,v) for k,v in bc.items()]
        nodes.sort(key=lambda x: x[1],reverse=True)
        node_key=nodes[15:25][random.randint(0,10)][0]
        g.remove_node(node_key)
        #print(nx.number_connected_components(g))
        for j in range(4):
            nx.write_weighted_edgelist(g,'edgelist/testdata/'+str(index))
            nx.write_weighted_edgelist(g, 'edgelist/testdata/' + str(40*2-1-index))
            index+=1

def generate_failure_network2():
    g= nx.read_weighted_edgelist('edgelist/ninux/0', nodetype=int)
    index=0
    for i in range(10):
        random.seed(1234)
        bc=nx.betweenness_centrality(g)
        nodes=[(k,v) for k,v in bc.items()]
        nodes.sort(key=lambda x: x[1],reverse=True)
        to_rem=0
        i=0
        for node in nodes:
            g1=g.copy()
            to_rem=node[0]
            g1.remove_node(node[0])
            if nx.is_connected(g1):
                print(i)
                break;
            i+=1
        g.remove_node(to_rem)
        #print(nx.number_connected_components(g))
        for j in range(4):
            nx.write_weighted_edgelist(g,'edgelist/testdata2/'+str(index))
            nx.write_weighted_edgelist(g, 'edgelist/testdata2/' + str(40*2-1-index))
            index+=1

def edges_to_graph(reference,maxsize=None,get_max_size=False):
    size=len(glob('edgelist/' + reference + '/*'))
    if not maxsize:
        maxsize=size
    x = list(range(maxsize))
    filenames=x
    ret_val=[]
    i=0
    for filename in filenames:
        print(str(round(100*i/maxsize,2))+"%")
        i+=1
        f = 'edgelist/' + reference + '/' + str(filename)
        g= nx.read_weighted_edgelist(f, nodetype=int)
        c = ComputeTheoreticalValues( graph=g)
        centrality=c.bet_dict
        m_bc=mean(list(centrality.values()))
        #m_bc=0
        biconnected = list(nx.biconnected_components(g))
        val={}
        val["std_bic"] = std([len(b) for b in biconnected])
        val["#bic"] = nx.number_connected_components(g)
        val["edge_std"]= std([e[2] for e in g.edges(data="weight")])
        val["#nodes"] = len(biconnected)
        Hi=c.Hi
        TCi=c.TCi
        Hi={k:v for k,v in Hi.items() if v>m_bc}
        TCi={k:v for k,v in TCi.items() if v>m_bc}
        ret_val.append((Hi,TCi,val))
    return ret_val

def compute_total_error(data,r=0):
    base=None
    error=0
    for i in data:
        if not base:
            base=i
        for k,v in i[0].items():
            if k in base[0]:
                error+=((v-base[0][k])/base[0][k])
            else:
                error+=1
    return error

def compute_error(data,stdd=0,nn=0,stde=0, nb=0,rand=False):
    base=None
    error=0
    rec=1
    first=False
    for i in data:
        if not base:
            base=i
        val=base[2]
        actual_val=i[2]
        stdda=(abs(val["std_bic"]-actual_val["std_bic"])/actual_val["std_bic"])
        nna=abs(val["#nodes"]-actual_val["#nodes"])
        stdea=abs((val["edge_std"]-actual_val["edge_std"])/actual_val["edge_std"])
        nba = abs(val["#bic"] - actual_val["#bic"])
        #print(stdda,nna,stdea)
        if first:
            if (not rand and stdda>=stdd and nna>=nn and stdea>=stde and nba>=nb) \
                    or (rand and randint(0,9)==0) :
                base = i
                rec+=1
        first=True
        for k,v in i[0].items():
            if k in base[0]:
                error+=((v-base[0][k])/base[0][k])
            else:
                error+=1
    return error,(100*rec)/len(data)

def compute_error_d(data,stdd=0,nn=0,stde=0, nb=0,rand=False):
    errs=[]
    recomputations=[]
    i=0
    for d in data:
        err,per_rec=compute_error(d,stdd=stdd,nn=nn,stde=stde,nb=nb,rand=rand)
        err=err/compute_total_error(d)
        errs.append(abs(err))
        recomputations.append(per_rec)
    return mean(errs),mean(recomputations)



def compute_error_d2(data,data_art,data_m,data_art2,stdd=0,nn=0,stde=0, nb=0,rand=False):
    print("all ",str(compute_error_d(data+data_art+data_m+data_art2,stdd=stdd,nn=nn,stde=stde,nb=nb,rand=rand)))
    return
    print("art ",str(compute_error_d(data_art+data_art2,stdd=stdd,nn=nn,stde=stde,nb=nb,rand=rand)))
    print("real ",str(compute_error_d(data+data_m,stdd=stdd,nn=nn,stde=stde,nb=nb,rand=rand)))
    print("imp ",str(compute_error_d(data_m+data_art2,stdd=stdd,nn=nn,stde=stde,nb=nb,rand=rand)))
    print("normn ",str(compute_error_d(data+data_art,stdd=stdd,nn=nn,stde=stde,nb=nb,rand=rand)))


"""
s=(edges_to_graph("testdata"))
text_file = open("td.txt", "w")
text_file.write(str(s))
text_file.close()
s=(edges_to_graph("testdata2"))
text_file = open("td2.txt", "w")
text_file.write(str(s))
text_file.close()
s=(edges_to_graph("ninux"))
text_file = open("ninux.txt", "w")
text_file.write(str(s))
text_file.close()
s=(edges_to_graph("FFGraz"))
text_file = open("FFGraz.txt", "w")
text_file.write(str(s))
text_file.close()
s=(edges_to_graph("FFWien"))
text_file = open("FFWien.txt", "w")
text_file.write(str(s))
text_file.close()
exit(0)
"""

data=""
with open('data_timers/td_n.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
testdata_n=eval(data)
with open('data_timers/td2_n.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
testdata2_n=eval(data)
with open('data_timers/ninux_n.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
ninux_n=eval(data)
with open('data_timers/FFGraz_n.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
FFGraz_n=eval(data)
with open('data_timers/FFWien_n.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
FFWien_n=eval(data)
with open('data_timers/td.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
testdata2=eval(data)
with open('data_timers/td2.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
testdata=eval(data)
with open('data_timers/ninux.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
ninux=eval(data)
with open('data_timers/FFGraz.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
FFGraz=eval(data)
with open('data_timers/FFWien.txt', 'r') as myfile:
    data=myfile.read().replace('\n', '')
FFWien=eval(data)

data=[FFGraz_n,FFWien_n,ninux_n]
data_art=[testdata2_n,testdata_n]
data_m=[FFGraz,FFWien,ninux]
data_art2=[testdata2,testdata]
compute_error_d2(data,data_art,data_m,data_art2, stdd=0.05,nn=4,stde=0.005,nb=0)
compute_error_d2(data,data_art,data_m,data_art2, rand=True)

