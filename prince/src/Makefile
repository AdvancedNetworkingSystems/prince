SRCS =  prince.c lib/ini.c parser.c

TARGET = prince
TARGET_C= prince_c
OBJS = $(SRCS:.c=.o)
OBJS_C = $(SRCS:.c=.o)
CFLAGS = -ansi -g -fPIC -D_GNU_SOURCE
CCFLAGS = --std=c++11 -g
LIBS = -L../../graph-parser/build/lib -lgraphparser -ldl -ljson-c
LIBC = -L../../graph-parser_c/build/lib -lgraphcparser -ldl -ljson-c
BPATH = ../build/

all: all_c
all_cpp: preparecpp $(TARGET)  libprince_olsr.so libprince_oonf.so removecomm
all_c: preparec $(TARGET_C)  libprince_olsr.so libprince_oonf.so removecomm

$(TARGET) : preparecpp $(OBJS)
	$(CXX) $(CCFLAGS) -o $(BPATH)$(TARGET) $(OBJS) $(LIBS)
$(TARGET_C) : preparec $(OBJS_C)
	$(CXX) $(CCFLAGS) -o $(BPATH)$(TARGET_C) $(OBJS) $(LIBC)


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
