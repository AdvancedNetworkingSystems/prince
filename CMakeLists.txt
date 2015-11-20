cmake_minimum_required(VERSION 3.3)

set (PROJECT_NAME "graph_parser" CXX)
project (${PROJECT_NAME})

set (PROJECT_SOURCE_DEFINITION_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
set (MAIN_FILE ${PROJECT_SOURCE_DEFINITION_DIRECTORY}/main.cpp)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set (BOOST_ROOT "/usr/local/boost/")
find_package(Boost)
if(Boost_FOUND)
    include_directories(${INCLUDE_DIRECTORIES} ${Boost_INCLUDE_DIRS})
    link_directories(${LINK_DIRECTORIES} ${Boost_LIBRARY_DIRS})
endif()

include_directories (${PROJECT_SOURCE_DECLARATION_DIRECTORY})

# "Add an executable to the project using the specified source files."
add_executable(${PROJECT_NAME} ${MAIN_FILE})

# "Link a target to given libraries."
target_link_libraries (${PROJECT_NAME})