SRCS =  prince.c lib/ini.c

TARGET = prince
OBJS = $(SRCS:.c=.o)
CFLAGS = -ansi -g -fPIC -D_GNU_SOURCE -DBOOST_LOG_DYN_LINK
CCFLAGS = --std=c++11 -g 
LIBS = -L../../graph-parser/build/lib -lgraphparser -ldl
BPATH = ../build/

all: $(TARGET) libprince_olsr.so libprince_oonf.so

$(TARGET) : $(OBJS)
	$(CXX) $(CCFLAGS) -o $(BPATH)$(TARGET) $(OBJS) $(LIBS)

libprince_olsr.so : olsr.o socket.o
	$(CC)  -shared -o  $(BPATH)$@  olsr.o socket.o

libprince_oonf.so : oonf.o socket.o
	$(CC)  -shared -o  $(BPATH)$@  oonf.o socket.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.so $(TARGET)
	$(RM) $(BPATH)*.o *.so $(BPATH)$(TARGET)
