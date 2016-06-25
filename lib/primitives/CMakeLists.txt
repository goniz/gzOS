cmake_minimum_required( VERSION 2.8 )

project(gzOS_libprimitives C CXX ASM)

add_library(primitives STATIC queue.cpp queue.h interrupts_guard.h spinlock_mutex.h spinlock_mutex.cpp lock_guard.h)
