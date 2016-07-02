cmake_minimum_required( VERSION 2.8 )

project(gzOS_pcnet C CXX ASM)

add_library(pcnet STATIC pcnet.cpp pcnet.h)
