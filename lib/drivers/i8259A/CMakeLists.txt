cmake_minimum_required( VERSION 2.8 )

project(gzOS_i8259A C CXX ASM)

add_library(i8259A STATIC i8259A.c i8259A.h)
