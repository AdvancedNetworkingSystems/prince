SRCS =  prince.c lib/ini.c parser.c

TARGET = prince
TARGET_C= prince_c
OBJS = $(SRCS:.c=.o)
CFLAGS = -ansi -g -fPIC -D_GNU_SOURCE -DBOOST_LOG_DYN_LINK
CCFLAGS = --std=c++11 -g 
CFLAGS =  -g 
LIBS = -L../../graph-parser/build/lib -lgraphparser -ldl -ljson-c
LIBS_C = -L../../graph-parser_c/build/lib -lgraphparser -ldl -ljson-c
BPATH = ../build/

all: $(TARGET) $(TARGET_C)  libprince_olsr.so libprince_oonf.so

$(TARGET) : $(OBJS)
	$(CXX) $(CCFLAGS) -o $(BPATH)$(TARGET) $(OBJS) $(LIBS)
$(TARGET_C) : $(OBJS)
	$(CXX) $(CFLAGS) -o $(BPATH)$(TARGET_C) $(OBJS) $(LIBS_C)

libprince_olsr.so : olsr.o socket.o parser.o
	$(CC)  -shared -o  $(BPATH)$@  olsr.o socket.o parser.o

libprince_oonf.so : oonf.o socket.o parser.o
	$(CC)  -shared -o  $(BPATH)$@  oonf.o socket.o parser.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.so $(TARGET)
	$(RM) $(BPATH)*.o *.so $(BPATH)$(TARGET)
	$(RM) lib/*.o
