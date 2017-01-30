CFLAGS+=-lm -ldl -ljson-c -pthread
prince:prince.o
	$(CC) $(LDFLAGS) biconnected.o graph.o ini.o network_change.o parser.o socket.o brandes.o graph_parser.o list.o oonf.o prince.o -o prince_c $(CFLAGS) 
prince.o:
	cp prince/src/common_c.h prince/src/common.h
	$(CC) -Dunique -g -c prince/src/prince.c prince/src/lib/ini.c prince/src/parser.c prince/src/oonf.c prince/src/socket.c graph-parser_c/src/brandes.c graph-parser_c/src/biconnected.c  graph-parser_c/src/graph_parser.c graph-parser_c/src/graph/graph.c graph-parser_c/src/graph/list.c graph-parser_c/src/network_change.c  $(CFLAGS)
	
clean:
	rm *.o prince
	rm prince/src/common.h
	
