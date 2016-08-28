cmake_minimum_required( VERSION 2.8 )

project(gzOS_network C CXX ASM)

add_library(network STATIC ethernet_layer.cpp ethernet_layer.h packet_pool.c packet_pool.h)
