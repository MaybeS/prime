cmake_minimum_required(VERSION 3.10)
project(prime)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fconcepts-ts")
set(CMAKE_CXX_FLAGS -pthread)

include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(prime main.cpp lib/pool.hpp lib/measure.hpp core/solver.hpp core/gridarray.hpp lib/progress.hpp)
