cmake_minimum_required(VERSION 3.15)
project(ex2_2)

set(CMAKE_CXX_STANDARD 14)

add_library(ex2_2 library.cpp library.h uthreads.cpp uthreads.h thread.cpp)