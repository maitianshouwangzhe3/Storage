cmake_minimum_required(VERSION 3.10.0)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

project(beggarvdb)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

include_directories(${CMAKE_SOURCE_DIR}/include)

aux_source_directory(src SRC)

add_executable(beggarvdb ${SRC}) #src/main.cpp src/ZSet.cpp src/hashtable.cpp src/db_server.cpp src/avltree.cpp