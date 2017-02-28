SRCS =  prince.c lib/ini.c parser.c
SRCS_C =  ../../graph-parser_c/src/brandes.c ../../graph-parser_c/src/biconnected.c  ../../graph-parser_c/src/graph_parser.c ../../graph-parser_c/src/graph/graph.c ../../graph-parser_c/src/graph/list.c
TARGET = prince
TARGET_C= prince_c
OBJS = $(SRCS:.c=.o)
OBJS_C = $(SRCS:.c=.o)
CFLAGS = -ansi -g -fPIC -D_GNU_SOURCE -DBOOST_LOG_DYN_LINK
CCFLAGS = --std=c++11 -g 
LIBS = -L../../graph-parser/build -lgraphparser -ldl -ljson-c
LIBC = -ldl -L../../graph-parser_c/build -lgraphparser -ljson-c -pthread
BPATH = ../build/

all: all_cpp removecomm
all_cpp: preparecpp $(TARGET)  libprince_olsr.so libprince_oonf.so 
#all_c: preparec $(TARGET_C)  libprince_olsr.so libprince_oonf.so 
$(TARGET) : preparecpp $(OBJS)
	$(CXX) $(CCFLAGS) -o $(BPATH)$(TARGET) $(OBJS) $(LIBS)
$(TARGET_C) : preparec $(OBJS_C)
	$(CXX) $(CCFLAGS) -o $(BPATH)$(TARGET_C) $(OBJS) $(LIBC)
	
c_no_lib:preparec
	gcc -Dunique -g -o ../build/prince_c prince.c lib/ini.c parser.c oonf.c olsr.c socket.c ../../graph-parser_c/src/brandes.c ../../graph-parser_c/src/biconnected.c  ../../graph-parser_c/src/graph_parser.c ../../graph-parser_c/src/graph/graph.c ../../graph-parser_c/src/graph/list.c ../../graph-parser_c/src/network_change.c -lm -ldl -ljson-c -pthread

libprince_olsr.so : olsr.o socket.o parser.o
	$(CXX)  -shared -fPIC -o  $(BPATH)$@  olsr.o socket.o parser.o

libprince_oonf.so : oonf.o socket.o parser.o
	$(CXX)  -shared -fPIC -o  $(BPATH)$@  oonf.o socket.o parser.o

preparec:
	cp common_c.h common.h
preparecpp:
	cp common_c++.h common.h
removecomm:
	rm common.h
clean:
	$(RM) *.o *.so $(TARGET)
	$(RM) $(BPATH)*.o *.so $(BPATH)$(TARGET)
	$(RM) lib/*.o
