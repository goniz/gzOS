cmake_minimum_required( VERSION 2.8 )

project(gzOS_network C CXX ASM)

add_library(network STATIC ethernet.cpp ethernet.h packet_pool.c packet_pool.h arp.cpp arp.h)
