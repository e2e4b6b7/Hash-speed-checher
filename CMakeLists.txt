cmake_minimum_required(VERSION 3.10)
project(HashChecker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Ofast -march=native")

add_executable(HashChecker main.cpp MurmurHash3.h xxhash.h SpeedTest.h Random.h Random.cpp)