cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS} -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -Wall -Werror=implicit-function-declaration -O2")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS} -Wall -Werror=implicit-function-declaration -O2 -g")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/prince")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

project(poprouting LANGUAGES C)

find_package(Threads)

add_executable(prince
        graph-parser_c/src/biconnected.c
        graph-parser_c/src/brandes.c
        graph-parser_c/src/graph/graph.c
        graph-parser_c/src/graph/list.c
        graph-parser_c/src/graph_parser.c
        graph-parser_c/src/network_change.c
        prince/src/config.c
        prince/src/config_proto.c
        prince/src/config_graph.c
        prince/src/load_plugin.c
        prince/src/prince.c
        prince/src/prince_handler.c
        prince/src/socket.c
        prince/src/topology.c
        prince/src/topology_parser.c
)

target_link_libraries(prince
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
        m
        json-c
)

include_directories(
        prince/include
        graph-parser_c/include
)

add_library(prince_oonf MODULE
        prince/lib/oonf/oonf.c
        prince/src/topology_parser.c
        prince/src/socket.c
        prince/src/topology.c
        prince/src/topology_parser.c
)

add_library(prince_olsr MODULE
        prince/lib/olsrd/olsr.c
        prince/src/socket.c
        prince/src/topology.c
        prince/src/topology_parser.c
)

add_library(prince_test MODULE
        prince/lib/testlib/test.c
        prince/src/topology_parser.c
        prince/src/socket.c
        prince/src/topology.c
        prince/src/topology_parser.c
)

install(TARGETS
        prince
        prince_oonf
        prince_olsr
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/prince
)

install(TARGETS
        prince
        prince_test
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/prince
        COMPONENT test
        EXCLUDE_FROM_ALL
)
