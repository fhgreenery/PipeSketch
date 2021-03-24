cmake_minimum_required(VERSION 3.17)
project(0715_newpcap)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(elastic)
include_directories(MVSketch)
include_directories(MVSketch2)

add_executable(0715_newpcap pipesketch.cpp}

