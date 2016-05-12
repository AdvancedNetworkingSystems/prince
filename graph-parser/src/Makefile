##############################################
# Makefile for hello-boost-graph program
##############################################

INCLUDES = -I/usr/local/boost/

# define the C source files
SRCS = bi_connected_components.cpp graph_manager.cpp sub_component.cpp utility.cpp graph_parser.cpp parser.cpp


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
MAIN_BIN = main

(MAIN): $(MAIN).o $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(MAIN_BIN) $(MAIN).o $(OBJS) $(LFLAGS) $(LIBS )
	$(CXX) $(CFLAGS) $(INCLUDES) -o  $(OBJS) $(LFLAGS) $(LIBS )

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES)  -std=c++11 -c $<  -o $@

# remove object files and executable when user executes "make clean"
clean:
	$(RM) *.o $(BIN_FILE)
