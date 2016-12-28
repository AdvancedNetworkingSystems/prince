all:
	mkdir -p prince/build/
	$(MAKE) -C graph-parser_c/src/
	$(MAKE) -C prince/src
	$(MAKE)  all_c -C prince/src

install:
	cp graph-parser_c/build/lib/libgraphcparser.so /usr/lib/
	cp prince/build/libprince_oonf.so /usr/lib/
	cp prince/build/prince_c /usr/bin/
uninstall:
	rm -f /usr/lib/libgraphcparser.so
	rm -f /usr/lib/libprince_oonf.so
	rm -f /usr/bin/prince_c
	rm -fr prince/build/
	rm -fr graph-parser_c/build/
	rm -fr graph-parser_c/src/*.o
	rm -fr graph-parser_c/src/*.~
