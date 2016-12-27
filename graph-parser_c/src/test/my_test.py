import networkx as nx
from random import *
r=Random(12345678)
from heapq import heappush, heappop
from itertools import count


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
        S, P, sigma = _single_source_dijkstra_path_basic(G, s, weight)
        print(s,P)
        # accumulation
        betweenness = _accumulate_basic(betweenness, S, P, sigma, s)
    # rescaling
    print(betweenness)
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
    while Q:
        (dist, _, pred, v) = pop(Q)
        #print("popping", v);
        if v in D:
            continue  # already searched this node.
        sigma[v] += sigma[pred]  # count paths
        S.append(v)
        D[v] = dist
        #print("size", len(G[v].items()));
        for w, edgedata in G[v].items():
            vw_dist = dist + edgedata.get(weight, 1)
            if w not in D and (w not in seen or vw_dist < seen[w]):
                seen[w] = vw_dist
                #print("pushing ",w,vw_dist);
                push(Q, (vw_dist, next(c), v, w))
                #print("pushed ",edgedata.get(weight, 1),w,Q);
                sigma[w] = 0.0
                P[w] = [v]
            elif vw_dist == seen[w]:  # handle equal paths
                sigma[w] += sigma[v]
                P[w].append(v)
    #print(S,P,sigma)

    return S, P, sigma


def _accumulate_basic(betweenness, S, P, sigma, s):
    delta = dict.fromkeys(S, 0)
    while S:
        w = S.pop()
        coeff = (1.0 + delta[w]) / sigma[w]
        for v in P[w]:
            delta[v] += sigma[v] * coeff
        if w != s:
            betweenness[w] += delta[w]
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
            print(scale)
    return betweenness
g=nx.fast_gnp_random_graph(5,0.75,seed=2,directed=True)
for (u, v) in g.edges():
    g.edge[u][v]['weight'] = r.randint(1,10)
#print(g.edges(data=True))
#print ("struct graph g;")
#print ("init_graph(&g);")
for u,v,a in g.edges(data=True):
    print ("add_edge_graph(&g,\""+str(u)+"\",\""+str(v)+"\","+str(a['weight'])+",0);")
#print nx.betweenness_centrality(g,weight='weight');
print betweenness_centrality(g,weight='weight');


