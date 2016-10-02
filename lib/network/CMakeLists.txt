cmake_minimum_required( VERSION 2.8 )

project(gzOS_network C CXX ASM)

add_library(network STATIC ethernet.cpp ethernet.h nbuf.c nbuf.h arp.cpp arp.h checksum.c checksum.h ip.cpp ip.h icmp.cpp icmp.h route.cpp route.h socket.cpp socket.h interface.cpp interface.h udp.cpp udp.h ip_raw.h)
