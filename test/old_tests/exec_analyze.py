import commands

def do_test(nodenum,is_c,heu):
	res =commands.getstatusoutput('perf stat -e cache-misses python -m memory_profiler  test_lib.py '+str(nodenum)+' '+str(is_c)+' '+str(heu))
	res=res[1]
	res=res.split("\n")
	data=eval(res[0])
	cpu_usage=data[1]
	micros=data[2]
	cache_misses=int(res[-4:-3][0].replace("cache-misses","").replace(".","").strip())

	ramM=res[-10:-9][0].replace("\tlib.compute();","").replace("MiB","").replace(" ","#")

	while ramM[-1:]=='#':
		ramM=ramM[:-1]
	r_usage=float(ramM[ramM.rfind("#")+1:])
	return (nodenum,round(cpu_usage,3),round(float(micros)/1000,3),"",cache_misses,"",r_usage)

f1 = open("res/c++.txt", "w+")
f1.truncate()
f1.write("nodenum\tcpu(%)\ttime(ms)\tcmisses(-)\tram(MiB)\n")
f2 = open("res/c++_h.txt", "w+")
f2.truncate()
f2.write("nodenum\tcpu(%)\ttime(ms)\tcmisses(-)\tram(MiB)\n")
f3 = open("res/c.txt", "w+")
f3.truncate()
f3.write("nodenum\tcpu(%)\ttime(ms)\tcmisses(-)\tram(MiB)\n")
f4 = open("res/c_h.txt", "w+")
f4.truncate()
f4.write("nodenum\tcpu(%)\ttime(ms)\tcmisses(-)\tram(MiB)\n")

fro=100
to=1200
every=25
repeats=30

for k in range(fro,to,every):
	for i in xrange(repeats):
		f1.write("\t".join(str(i) for i in do_test(k,0,0))+"\n")
print("ended 0,0")
for k in range(fro,to,every):
	for i in xrange(repeats):
		f2.write("\t".join(str(i) for i in do_test(k,1,0))+"\n")
print("ended 1,0")
for k in range(fro,to,every):
	for i in xrange(repeats):
		f3.write("\t".join(str(i) for i in do_test(k,0,1))+"\n")
print("ended 0,1")
for k in range(fro,to,every):
	for i in xrange(repeats):
		f4.write("\t".join(str(i) for i in do_test(k,1,1))+"\n")
print("ended 1,1")
f1.close()
f2.close()
f3.close()
f4.close()


