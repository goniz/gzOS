cmake_minimum_required( VERSION 2.8 )

project(gzOS_libmalloc C CXX ASM)

add_library(malloc STATIC malloc.c ../syscall/syscall.h)
