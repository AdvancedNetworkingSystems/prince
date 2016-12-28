SRCS =  prince.c lib/ini.c parser.c

TARGET = prince
TARGET_C= prince_c
OBJS = $(SRCS:.c=.o)
OBJS_C = $(SRCS:.c=.o)
CFLAGS = -std=gnu11 -g -fPIC -D_GNU_SOURCE
LIBS = -L../../graph-parser/build/lib -lgraphparser -ldl -ljson-c
LIBC = -L../../graph-parser_c/build/lib -lgraphcparser -ldl -ljson-c -lm
BPATH = ../build/

all: all_c
all_c: preparec $(TARGET_C)  libprince_oonf.so removecomm

$(TARGET_C) : preparec $(OBJS_C)
	$(CC) $(CFLAGS) -o $(BPATH)$(TARGET_C) $(OBJS) $(LIBC)


libprince_oonf.so : oonf.o socket.o parser.o
	$(CC)  -shared -fPIC -o  $(BPATH)$@  oonf.o socket.o parser.o

preparec:
	cp common_c.h common.h
removecomm:
	rm common.h
clean:
	$(RM) *.o *.so $(TARGET)
	$(RM) $(BPATH)*.o *.so $(BPATH)$(TARGET)
	$(RM) lib/*.o
