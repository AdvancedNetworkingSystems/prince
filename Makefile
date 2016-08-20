all:
	$(MAKE) -C graph-parser/src
	$(MAKE) -C prince/src

install:
	cp graph-parser/build/lib/libgraphparser.so /usr/lib/
	cp prince/build/libprince_oonf.so /usr/lib/
	cp prince/build/prince /usr/bin/

uninstall: 
	rm /usr/lib/libgraphparser.so
	rm /usr/lib/libprince_oonf.so
	rm /usr/bin/prince
