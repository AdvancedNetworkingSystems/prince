import numpy.random as rnd
import scipy
import networkx as nx
import numpy as np
import collections


def compute_eclidean_sqdistance(n1, n2):
    """ compute the squared euclidean distance between two nodes """
    return pow(n1[0]-n2[0], 2) + pow(n1[1]-n2[1], 2)


def truncated_integer_power_law(nmax, samples, alpha=2.71):
    """ produce a sample of degree sequence from a truncated
    integer power law distribution"""
    x = np.arange(1, nmax+1, dtype=float)
    pdf = 1/x**alpha
    pdf /= sum(pdf)
    rg = scipy.stats.rv_discrete(values=(range(1, nmax+1), pdf))
    return rg.rvs(size=samples)


class MeshGenerator():
    def __init__(self, N, R=100, num_retries=20, seed=False):
        """ A mesh network generator based on the paper:
        NPART - Node Placement Algorithm for Realistic Topologies in Wireless
        Multihop Network Simulation. From  Bratislav Milic and Miroslaw Malek
        {milic, malek}@informatik.hu-berlin.de.

        N: number of nodes in the network
        R: communication radius"""
        self.N = N
        self.num_retries = num_retries
        self.R = R
        if seed:
            rnd.seed(seed)
        # TODO: use link lenght distribution from "Topoogy patterns"
        # paper from guifi

    def place_nodes(self):
        self.graph = nx.Graph()
        x = rnd.uniform(1000)
        y = rnd.uniform(1000)
        self.graph.add_node(0, {"x": x, "y": y})
        min_x = x
        min_y = y
        max_x = x
        max_y = y
        placed_nodes = 1
        self.deg_distribution = self.create_degree_distribution()
        while (placed_nodes < self.N):
            evalued_candidates = 0
            min_metric = 1000000  # just a very big value
            best_candidate = []
            while (evalued_candidates < self.num_retries):
                neighs = {}
                while True:
                    repeatitions = 0
                    x = rnd.uniform(min_x - self.R, max_x + self.R)
                    y = rnd.uniform(min_y - self.R, max_y + self.R)
                    neighs = self.compute_neighbors([x, y])
                    if neighs:
                        break
                    if repeatitions > 1000:
                        print "You have a problem with the chosen radius"
                        print self.R, "I could not find a connected node"
                        print "in 1000 attempts"
                        exit(1)
                    repeatitions += 1
                candidate_metric = self.compute_metric(neighs)
                if candidate_metric < min_metric:
                    min_metric = candidate_metric
                    best_candidate = [x, y, neighs]
                evalued_candidates += 1
            new_node_id = placed_nodes
            self.graph.add_node(new_node_id, {"x": x, "y": y})
            for n in best_candidate[2].keys():
                self.graph.add_edge(new_node_id, n)
            placed_nodes += 1
            min_x = min(min_x, x)
            min_y = min(min_y, y)
            max_x = max(max_x, x)
            max_y = max(max_y, y)

    def create_degree_distribution(self):
        """ create a random degree distribution from power-law distribution
        guifi uses alpha =2.71 (see "Topology Patterns of a Community Network:
        Guifi.net") which creates too many leaves. The first parameter on the
        power law function determines the maximum degree achievable"""
        deg_array = sorted(truncated_integer_power_law(self.N/7,
                           self.N, alpha=1.6))
        dist = collections.Counter(deg_array)
        for i in range(1, max(dist)):
            if i not in dist:
                dist[i] = 0
        return dist

    def compute_neighbors(self, candidate, radius=0):
        """ list the nodes that can be connected to the candidate,
        if needed, specify a radius for the node """

        if not radius:
            sq_radius = self.R*self.R
        else:
            sq_radius = radius*radius
        neighbor_nodes = {}
        for (node, attrs) in self.graph.nodes(data=True):
            d = compute_eclidean_sqdistance((attrs["x"], attrs["y"]),
                                            candidate)
            if d < sq_radius:
                neighbor_nodes[node] = pow(d, 0.5)
        return neighbor_nodes

    def compute_metric(self, neighs, metric="ADAPTIVE"):
        if metric == "ADAPTIVE":
            return self.adaptive_metric(neighs)
        else:
            "ERROR: did not implemented other metric than ADAPTIVE"
        exit(1)

    def adaptive_metric(self, neighs, p=5):
        """ Compute the adaptive metric for the candidate node """
        temp_g = self.graph.copy()
        weights = collections.Counter(temp_g.degree().values())
        newnode = len(temp_g)  # index of a new node
        temp_g.add_node(newnode)
        for i in neighs:
            temp_g.add_edge(newnode, i)
        candidate_frequency = collections.Counter(temp_g.degree().values())
        for deg in self.deg_distribution:
            if deg not in candidate_frequency:
                candidate_frequency[deg] = 0
        weights.subtract(self.deg_distribution)
        weights_sum = sum(map(abs, weights.values()))
        for k, v in weights.items():
            weights[k] = abs(v)/weights_sum
        metric = 0
        for (d, v) in self.deg_distribution.items():
            candidate_diff = v - candidate_frequency[d]
            if candidate_diff < 0:
                metric += abs(candidate_diff)*p
            else:
                metric += candidate_diff*weights[d]
        return metric
