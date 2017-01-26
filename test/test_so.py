from ctypes import cdll,c_double,py_object
from graph_generator import Gen
import json
import datetime as dt
import networkx as nx
import numpy as np
import matplotlib.pyplot as plt

def run_tests(node_num,repeat,lib_n,w):
	ge = Gen()
	executions = []
	retval={}
	for i in xrange(repeat):
		ge.genGraph("PLAW", node_num)
		g=ge.graph
		s=json.dumps(ge.composeNetJson(g))
		lib = cdll.LoadLibrary(lib_n)
		lib.get_res.restype=py_object
		lib.init(w);
		lib.parse(s);

		n1=dt.datetime.now()
		lib.compute();
		elapsed_time = (dt.datetime.now()-n1).microseconds
		elapsed_time=elapsed_time/1000
		res=lib.get_res()
		lib.destroy();
		executions.append(elapsed_time)
	print(executions)
	return (np.mean(executions),np.var(executions))
x=[]
var_c=[]
m_c=[]
var_cpp=[]
m_cpp=[]
var_c_heu=[]
m_c_heu=[]
var_cpp_heu=[]
m_cpp_heu=[]
repeats=4

for k in range(50,801,50):
	print(k)
	r=run_tests(k,repeats,'./libtest_c.so',0)
	m_c.append(r[0])
	var_c.append(r[1])
	r=run_tests(k,repeats,'./libtest_cpp.so',0)
	m_cpp.append(r[0])
	var_cpp.append(r[1])
	r=run_tests(k,repeats,'./libtest_c.so',1)
	m_c_heu.append(r[0])
	var_c_heu.append(r[1])
	r=run_tests(k,repeats,'./libtest_cpp.so',1)
	m_cpp_heu.append(r[0])
	var_cpp_heu.append(r[1])
	x.append(k)
print(m_cpp,m_cpp_heu)

plt.gca().set_position((.1, .2, .8, .7))
plt.gca().set_xlim( (0,1200) )
"""
plt.errorbar(x, m_cpp, yerr=var_cpp, label="C++")
plt.errorbar(x, m_cpp_heu, yerr=var_cpp_heu, label="C++ heuristic")
plt.errorbar(x, m_c, yerr=var_c, label="C")
plt.errorbar(x,m_c_heu, yerr=var_c_heu, label="C heuristic")
"""
plt.errorbar(x, m_cpp,  label="C++")
plt.errorbar(x, m_cpp_heu,  label="C++ heuristic")
plt.errorbar(x, m_c,  label="C")
plt.errorbar(x, m_c_heu,  label="C heuristic")
plt.xlabel('size of graph (nodes)', fontsize=16)
plt.ylabel('execution time (ms)', fontsize=16)
plt.legend(loc='upper left', shadow=True)
plt.figtext(.4, .02, "# samples= "+str(repeats), fontsize=16)
#plt.savefig('foo.pdf')
plt.show()


