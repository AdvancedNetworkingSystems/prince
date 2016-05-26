SRCS = olsr.c oonf.c common.c prince.c 
TARGET = prince
OBJS = $(SRCS:.c=.o)
CFLAGS = -g
CCFLAGS = --std=c++11 -g
LIBS = -L../../graph-parser/build/lib -lgraphparser
BPATH = ../build/

$(TARGET) : $(OBJS)
	$(CXX) $(CCFLAGS) -o $(BPATH)$(TARGET) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o $(TARGET)
	$(RM) $(BPATH)*.o $(BPATH)$(TARGET)
