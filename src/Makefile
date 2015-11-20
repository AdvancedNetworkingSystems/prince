##############################################
# Makefile for hello-boost-graph program
##############################################

BOOST_ROOT="/usr/local/boost/"

main: main.o
	$(CXX) $(LDFLAGS) main.o -o ../bin/main

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -I $(BOOST_ROOT) -std=c++11 -c main.cpp $(LIBS)

# remove object files and executable when user executes "make clean"
clean:
	rm *.o ./bin/main