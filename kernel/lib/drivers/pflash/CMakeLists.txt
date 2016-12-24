cmake_minimum_required( VERSION 2.8 )

project(gzOS_pflash C CXX ASM)

add_library(pflash STATIC "" pflash_fd.cpp pflash_fd.h pflash_fs.cpp pflash_fs.h)
