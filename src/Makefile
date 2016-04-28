##############################################
# Makefile for hello-boost-graph program
##############################################

INCLUDES = -I/usr/local/boost/

# define the C source files
SRCS = bi_connected_components.cpp graph_manager.cpp parser.cpp sub_component.cpp utility.cpp


# define the C object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.cpp=.o)

MAIN = main
MAIN_BIN = graph-parser
SIMULATION = simulation

# all: ../bin/$(MAIN)
# 	@echo  Simple compiler named main has been compiled

$(MAIN): $(MAIN).o $(SIMULATION).o $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(MAIN_BIN) $(MAIN).o $(OBJS) $(LFLAGS) $(LIBS )
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(SIMULATION) $(SIMULATION).o $(OBJS) $(LFLAGS) $(LIBS )


$(SIMULATION): $(SIMULATION).o $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(SIMULATION) $(SIMULATION).o $(OBJS) $(LFLAGS) $(LIBS )

countbcc: count_bcc.o $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -o countbcc count_bcc.o $(OBJS) $(LFLAGS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) -std=c++11 -c $<  -o $@

# remove object files and executable when user executes "make clean"
clean:
	$(RM) *.o $(BIN_FILE)