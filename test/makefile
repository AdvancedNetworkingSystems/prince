compile: compile_c #compile_cpp
compile_cpp:
	g++ -std=c++11 -shared -Wl,-soname,libtest_cpp.so -fPIC -o libtest_cpp.so  test.cpp ../graph-parser/src/betweenness_centrality.cpp ../graph-parser/src/parser.cpp ../graph-parser/src/betweenness_centrality_heuristic.cpp  ../graph-parser/src/sub_component.cpp ../graph-parser/src/graph_manager.cpp ../graph-parser/src/utility.cpp ../graph-parser/src/graph_parser.cpp ../prince/src/parser.c -ljson-c
compile_c:
	g++ -shared -Wl,-soname,libtest_c.so -fPIC -o libtest_c.so  test.cpp ../graph-parser_c/src/graph_parser.c ../graph-parser_c/src/algorithms.c ../graph-parser_c/src/graph/graph.c ../graph-parser_c/src/graph/list.c ../prince/src/parser.c -ljson-c -pthread



