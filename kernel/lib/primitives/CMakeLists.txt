cmake_minimum_required( VERSION 2.8 )

project(gzOS_libprimitives C CXX ASM)

add_library(primitives STATIC basic_queue.cpp basic_queue.h interrupts_mutex.h spinlock_mutex.h spinlock_mutex.cpp lock_guard.h Suspendable.cpp Suspendable.h Event.cpp Event.h hexdump.c hexdump.h hashmap.c hashmap.h array.h)
