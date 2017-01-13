import numpy as np
import matplotlib.pyplot as plt
def f(f_name):
	with open(f_name) as f:
	    content = f.readlines()[1:]
	content=[l.split('\t') for l in content]
	content=[[int(l[0]),float(l[1]),float(l[2]),int(l[4]),float(l[6])] for l in content]
	res={}
	res["#nodes"]=[]
	res["time"]=[]
	res["cmisses"]=[]
	res["ram"]=[]
	res["cpu"]=[]
	res["time_v"]=[]
	res["cmisses_v"]=[]
	res["ram_v"]=[]
	res["cpu_v"]=[]
	s=-1
	time=[]
	cmisses=[]
	ram=[]
	cpu=[]
	cut_variance=False
	for c in content:
		if s!=c[0]:
			if s>0:
				res["#nodes"].append(s)
				res["time"].append(int(np.mean(time)))
				res["cmisses"].append(np.mean(cmisses))
				res["ram"].append(np.mean(ram))
				res["cpu"].append(np.mean(cpu))
				v=np.var(time)
				if v>100 and cut_variance:
					v=100
				res["time_v"].append(v)
				v=np.var(cmisses)
				if v>100 and cut_variance:
					v=100
				res["cmisses_v"].append(v)
				v=np.var(ram)
				if v>100 and cut_variance:
					v=100
				elif v>0.00001 and cut_variance:
					v=0
				res["ram_v"].append(v)
				v=np.var(cpu)
				if v>0.3 and cut_variance:
					v=0.3
				res["cpu_v"].append(v)
			s=c[0]
			time=[]
			cmisses=[]
			ram=[]
		if s==c[0]:
			time.append(c[2])
			cmisses.append(c[3])
			ram.append(c[4])
			cpu.append(c[1])
	return res


r_c=f("c.txt")
r_ch=f("c_h.txt")
r_cpp=f("c++.txt")
r_cpph=f("c++_h.txt")


def make_graph(name_m,name_v,f_n,label):
	plt.gca().set_position((.1, .2, .8, .7))
	plt.gca().set_xlim( (0,1300) )
	print(name_m)
	print(r_cpp[name_v])
	print(r_cpph[name_v])
	print(r_c[name_v])
	print(r_ch[name_v])
	plt.errorbar(r_c["#nodes"], r_cpp[name_m], yerr=r_cpp[name_v],  label="C++")
	plt.errorbar(r_c["#nodes"], r_cpph[name_m], yerr=r_cpph[name_v], label="C++ heuristic")
	plt.errorbar(r_c["#nodes"], r_c[name_m], yerr=r_c[name_v], label="C")
	plt.errorbar(r_c["#nodes"],  r_ch[name_m], yerr= r_ch[name_v], label="C heuristic")
	plt.xlabel('size of graph (nodes)', fontsize=16)
	plt.ylabel(label, fontsize=16)
	plt.legend(loc='upper left', shadow=True)
	plt.figtext(.4, .02, "# samples= 30", fontsize=16)
	plt.savefig(f_n)
	#plt.show()
	plt.clf()
	

make_graph("time","time_v",'time.pdf','execution time (ms)')
make_graph("cmisses","cmisses_v",'misses.pdf','cache misses (#)')
make_graph("ram","ram_v",'ram.pdf','ram (Mib)')
make_graph("cpu","cpu_v",'cpu.pdf','cpu (%)')

		
