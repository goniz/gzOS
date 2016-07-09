cmake_minimum_required( VERSION 2.8 )

project(gzOS_e1000 C CXX ASM)

add_library(e1000 STATIC e1000.cpp e1000.h)
