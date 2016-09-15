##############################################
# Makefile for hello-boost-graph program
##############################################

# define the C source files
SRCS = betweenness_centrality_heuristic.cpp betweenness_centrality.cpp graph_manager.cpp sub_component.cpp utility.cpp parser.cpp graph_parser.cpp
TARGET = libgraphparser
OBJS = $(SRCS:.cpp=.o)
CXXFLAGS = -std=c++11 -fPIC -g
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

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $(BPATH)$(TARGET).so $(OBJS)  $(LIBS )


log: CXXFLAGS = -std=c++11 -fPIC -g
log: LIBS = -lpthread 
log: $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $(BPATH)$(TARGET).so $(OBJS)  $(LIBS )


# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) $(CXXFLAGS) -c $<  -o $@

# remove object files and executable when user executes "make clean"
clean:
	$(RM) *.o $(BPATH)$(TARGET).so
