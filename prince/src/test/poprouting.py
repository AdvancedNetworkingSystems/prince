#!/usr/bin/env python
import networkx as nx
import math


class ComputeTheoreticalValues():

    def __init__(self, graph, cent="B", cH=2.0, cTC=5.0):
        self.G = graph
        self.cent = cent
        self.cH = cH
        self.cTC = cTC
        self.decimal_values = 3
        if cent == "B":
            self.bet_dict = nx.betweenness_centrality(self.G, endpoints=True)
            self.bet_ordered_nodes = [i[0] for i in sorted(
                self.bet_dict.items(), key=lambda x: x[1])]
        elif cent == "C":
            self.bet_dict = nx.betweenness_centrality(self.G, endpoints=True)
            self.bet_ordered_nodes = [i[0] for i in sorted(
                self.bet_dict.items(), key=lambda x: x[1])]

            self.cent_dict = nx.closeness_centrality(self.G)
            self.cent_ordered_nodes = [i[0] for i in sorted(
                self.cent_dict.items(), key=lambda x: x[1])]
        self.deg_dict = self.G.degree()
        self.node_list = filter(lambda x: self.deg_dict[x] > 0, self.G)
        self.R = len(self.G.edges())
        self.compute_constants()
        # self.print_constants()
        self.compute_timers()
        # self.check_consistency()

    def compute_constants(self):
        # self.lambda_H, self.lambda_TC, self.O_H, self.O_TC = \
        self.O_H = sum([self.deg_dict[l] for l in self.node_list])/self.cH
        self.O_TC = len(self.node_list)*self.R/self.cTC
        sqrt_sum = 0
        for node in self.node_list:
            sqrt_sum += math.sqrt(self.deg_dict[node]*self.bet_dict[node])
        self.sq_lambda_H = sqrt_sum/self.O_H
        sqrt_sum = 0
        for node in self.node_list:
            sqrt_sum += math.sqrt(self.R*self.bet_dict[node])
        self.sq_lambda_TC = sqrt_sum/self.O_TC

    def print_constants(self):
        print "Loaded Graph with:"
        print len(self.G.nodes()), " nodes"
        print len(self.G.edges()), " edges"
        print self.O_H, self.O_TC, self.sq_lambda_H, self.sq_lambda_TC

    def print_timers(self):
        print "H:", [h[1] for h in sorted(self.Hi.items(), key=lambda x:x[1])]
        print "TC:", \
            [t[1] for t in sorted(self.TCi.items(), key=lambda x:x[1])]
        vv =  sorted([[self.deg_dict[node]/self.Hi[node], self.bet_dict[node],
                            self.deg_dict[node], self.Hi[node], self.TCi[node]] for node in self.node_list], key=lambda x:-x[0])
        for v in vv:
                print v

    def get_graph_size(self):
        return len(self.G.nodes())

    def compute_timers(self):
        self.Hi = {}
        self.TCi = {}
        for node in self.node_list:
            # print str(node) + "  " + str(self.bet_dict[node])
            self.Hi[node] = \
                    math.sqrt(self.deg_dict[node] / self.bet_dict[node]) * self.sq_lambda_H
            self.TCi[node] = \
                math.sqrt(self.R / self.bet_dict[node]) * self.sq_lambda_TC

    def compute_average_load(self, pop=True):
        O_h = 0
        O_tc = 0
        if pop:
            for node in self.node_list:
                O_h += self.deg_dict[node] / self.Hi[node]
                O_tc += self.R / self.TCi[node]
        else:
            for node in self.node_list:
                O_h += self.deg_dict[node] / self.cH
                O_tc += self.R / self.cTC
        return round(O_h, self.decimal_values), \
            round(O_tc, self.decimal_values)

    def compute_average_loss(self, pop=True, leaf_nodes=True):
        L_h = 0
        L_tc = 0
        for node in self.node_list:
            if not leaf_nodes:
                if self.deg_dict[node] == 1:
                    continue
            if pop:
                L_h += self.Hi[node] * self.bet_dict[node]
                L_tc += self.TCi[node] * self.bet_dict[node]
            else:
                L_h += self.cH * self.bet_dict[node]
                L_tc += self.cTC * self.bet_dict[node]
        return round(L_h, self.decimal_values), \
            round(L_tc, self.decimal_values)

    def check_consistency(self):
        """ Check that the generated overhead is the same with
        and without pop """
        load = self.compute_average_load(pop=True), \
            self.compute_average_load(pop=False)
        loss = self.compute_average_loss(pop=True), \
            self.compute_average_loss(pop=False)

        print "Load pop/standard", load[0][0] / load[1][0], load[0][1] / load[1][1]
        print "Loss pop/standard", 1 - loss[0][0] / loss[1][0], \
            1 - loss[0][1] / loss[1][1]
