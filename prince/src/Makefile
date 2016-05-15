SRCS = prince.c
TARGET = prince
OBJS = $(SRCS:.cpp = .o)
CPPOBJ = ../../graph-parser/src/betweenness_centrality.o ../../graph-parser/src/betweenness_centrality_heuristic.o ../../graph-parser/src/graph_manager.o 
CFLAGS = -g
LIBS = -L../../graph-parser/src -lgraphparser


(TARGET) : $(OBJS)
	$(CXX) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o $(TARGET)
