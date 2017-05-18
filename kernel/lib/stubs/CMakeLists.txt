cmake_minimum_required(VERSION 2.8)

project(gzOS_libstubs)

add_library(stubs STATIC cstubs.cpp cppstubs.cpp malloc.cpp)
