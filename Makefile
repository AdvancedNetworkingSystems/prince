CFLAGS+=-lm -ldl -ljson-c -pthread
poprouting:out libs cpp
	cp prince/src/common_c.h prince/src/common.h
	$(CC) $(LDFLAGS) -Dunique prince/src/prince.c prince/src/lib/ini.c prince/src/parser.c prince/src/socket.c graph-parser_c/src/brandes.c graph-parser_c/src/biconnected.c  graph-parser_c/src/graph_parser.c graph-parser_c/src/graph/graph.c graph-parser_c/src/graph/list.c graph-parser_c/src/network_change.c -o output/prince_c   $(CFLAGS)
libs:
	cp prince/src/common_c.h prince/src/common.h
	$(CC)  -shared -fPIC -o  output/libprince_olsr_c.so  prince/src/olsr.c prince/src/socket.c prince/src/parser.c
	$(CC)  -shared -fPIC -o  output/libprince_oonf_c.so prince/src/oonf.c prince/src/socket.c prince/src/parser.c
	rm prince/src/common.h
cpp:
	$(MAKE) -C graph-parser/src
	$(MAKE) -C prince/src
	 cp prince/build/* output/
clean:
	rm *.o
out:
	mkdir -p output
	mkdir -p prince/build/
	mkdir -p graph-parser/build/
	
	
