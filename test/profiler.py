import psutil
import threading
from time import sleep
import json
import numpy as np
from ctypes import cdll,c_double,py_object
from graph_generator import Gen
import datetime as dt
import networkx as nx

class CPUThread(threading.Thread):
	def __init__(self):
		super(CPUThread, self).__init__()
		self.usage=[]
		self.active=True
	def run(self):
		while self.active:
			self.usage.append(psutil.cpu_percent(interval=1))
			sleep(1)

	
def run_fun(f,n,h,w):
	t=CPUThread()
	t.start()
	elapsed_time=fun(n,h,w)
	t.active=False;
	t.join()
	print("data",np.mean(t.usage),elapsed_time)
		

#@profile
def compute(lib):
	lib.compute();

def fun(node_num,h,w):
		ge = Gen()
		ge.genGraph("PLAW", node_num)
		g=ge.graph

		s=json.dumps(ge.composeNetJson(g))
		lib_n='./libtest_cpp.so'
		if h==1:
			lib_n='./libtest_c.so'
		lib = cdll.LoadLibrary(lib_n)
		lib.get_res.restype=py_object
		lib.init(w);
		lib.parse(s);
		n1=dt.datetime.now()
		compute(lib)
		elapsed_time = (dt.datetime.now()-n1).microseconds
		elapsed_time=elapsed_time
		res=lib.get_res()

		print(dict(map(lambda (k,v): (k, round(v,8)), nx.betweenness_centrality(g,endpoints=True).iteritems()))==dict(map(lambda (k,v): (k, round(v,8)), res.iteritems())))
		print(nx.betweenness_centrality(g,endpoints=True))
		print(res)
		lib.destroy();
		return elapsed_time


import sys
args=sys.argv[1:]
#nodenum=int(args[0])
#is_c=int(args[1])
#heu=int(args[2])
nodenum=1000
is_c=1
heu=1
run_fun(fun,nodenum,is_c,heu)
nodenum=20
is_c=1
heu=0
run_fun(fun,nodenum,is_c,heu)


