cmake_minimum_required( VERSION 2.8 )

project(gzOS_libsyscall C CXX ASM)

add_library(syscall STATIC syscall.h syscall.c)
