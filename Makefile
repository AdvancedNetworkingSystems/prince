CFLAGS+=-lm -ldl -ljson-c -pthread
poprouting:out libs
	$(CC) -g $(LDFLAGS) prince/src/prince.c prince/src/config.c prince/src/parser.c prince/src/socket.c graph-parser_c/src/brandes.c graph-parser_c/src/biconnected.c  graph-parser_c/src/graph_parser.c graph-parser_c/src/graph/graph.c graph-parser_c/src/graph/list.c graph-parser_c/src/network_change.c -o output/prince   $(CFLAGS)
libs:
	$(CC) -g -shared -fPIC -o output/libprince_oonf.so prince/src/olsr/oonf.c prince/src/socket.c prince/src/parser.c
	$(CC) -g -shared -fPIC -o output/libprince_ospf.so prince/src/ospf/ospf.c prince/src/socket.c prince/src/parser.c
	$(CC) -g -shared -fPIC -o output/libprince_olsr.so prince/src/olsr/olsr.c prince/src/socket.c prince/src/parser.c


clean:
	rm output/*

out:
	mkdir -p output
	mkdir -p prince/build/
install:
	cp output/libprince_oonf.so /usr/lib/
	cp output/libprince_olsr.so /usr/lib/
	cp output/libprince_ospf.so /usr/lib/
	cp output/prince /usr/bin/

uninstall:
	rm -f /usr/lib/libprince_oonf.so
	rm -f /usr/lib/libprince_olsr.so
	rm -f /usr/lib/libprince_ospf.so
	rm -f /usr/bin/prince
	rm -fr prince/build/
