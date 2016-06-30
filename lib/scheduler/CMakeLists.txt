cmake_minimum_required( VERSION 2.8 )

project(gzOS_libscheduler C CXX ASM)

add_library(scheduler STATIC scheduler.cpp process.h process.cpp signals.h)
