CFLAGS+=-lm -ldl -ljson-c -pthread
poprouting:out libs
	cp prince/src/common_c.h prince/src/common.h
	$(CC) $(LDFLAGS) prince/src/prince.c prince/src/lib/ini.c prince/src/parser.c prince/src/socket.c graph-parser_c/src/brandes.c graph-parser_c/src/biconnected.c  graph-parser_c/src/graph_parser.c graph-parser_c/src/graph/graph.c graph-parser_c/src/graph/list.c graph-parser_c/src/network_change.c -o output/prince   $(CFLAGS)
	rm prince/src/common.h
libs:
	cp prince/src/common_c.h prince/src/common.h
	$(CC) -shared -fPIC -o output/libprince_oonf_c.so prince/src/oonf.c prince/src/socket.c prince/src/parser.c graph-parser_c/src/brandes.c graph-parser_c/src/biconnected.c  graph-parser_c/src/graph_parser.c graph-parser_c/src/graph/graph.c graph-parser_c/src/graph/list.c graph-parser_c/src/network_change.c
clean:
	rm output/*

out:
	mkdir -p output
	mkdir -p prince/build/
install:
	cp output/libprince_oonf_c.so /usr/lib/
	cp output/prince /usr/bin/

uninstall:
	rm -f /usr/lib/libprince_oonf_c.so
	rm -f /usr/bin/prince
	rm -fr prince/build/
