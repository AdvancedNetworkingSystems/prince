CFLAGS+=-lm -ldl -ljson-c -pthread
poprouting:
	cp prince/src/common_c.h prince/src/common.h
	$(CC) $(LDFLAGS) -Dunique prince/src/prince.c prince/src/lib/ini.c prince/src/parser.c prince/src/oonf.c prince/src/olsr.c prince/src/socket.c graph-parser_c/src/brandes.c graph-parser_c/src/biconnected.c  graph-parser_c/src/graph_parser.c graph-parser_c/src/graph/graph.c graph-parser_c/src/graph/list.c graph-parser_c/src/network_change.c -o prince_c   $(CFLAGS)
	
clean:
	rm *.o prince
	rm prince/src/common.h
	
