
# define the C source files
SRCS = graph_parser.c brandes.c biconnected.c graph/graph.c graph/list.c
TARGET = libgraphcparser
OBJS = $(SRCS:.c=.o)
CFLAGS = -fPIC -pthread
LDFLAGS = -shared -Wl,-soname,$(TARGET).so
BPATH = ../build/lib/


# define the C object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#

$(TARGET): makefolder $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $(BPATH)$(TARGET).so $(OBJS)  $(LIBS )


log: CFLAGS = -fPIC -g -pthread
log: LIBS = -lpthread 
log: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $(BPATH)$(TARGET).so $(OBJS)  $(LIBS )

makefolder:
	mkdir -p $(BPATH)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(C) $(CFLAGS) $(INCLUDES) $(CFLAGS) -c $<  -o $@

# remove object files and executable when user executes "make clean"
clean:
	$(RM) *.o $(BPATH)$(TARGET).so
