all:
	mkdir -p graph-parser/build/lib
	mkdir -p prince/build/
	$(MAKE) -C graph-parser/src
	$(MAKE) -C prince/src

install:
	cp graph-parser/build/lib/libgraphparser.so /usr/lib/
	cp prince/build/libprince_oonf.so /usr/lib/
	cp prince/build/prince /usr/bin/

uninstall: 
	rm -f /usr/lib/libgraphparser.so
	rm -f /usr/lib/libprince_oonf.so
	rm -f /usr/bin/prince
	rm -fr prince/build/
	rm -fr graph-parser/build/

