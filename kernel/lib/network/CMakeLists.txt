cmake_minimum_required( VERSION 2.8 )

project(gzOS_network C CXX ASM)

add_library(network STATIC ethernet/ethernet.cpp ethernet/ethernet.h nbuf.c nbuf.h arp/arp.cpp arp/arp.h checksum.c checksum.h ip/ip.cpp ip/ip.h icmp/icmp.cpp icmp/icmp.h route.cpp route.h socket.cpp socket.h interface.cpp interface.h udp/udp.cpp udp/udp.h ip/ip_raw.h syscalls.cpp udp/udp_socket.cpp udp/udp_socket.h tcp/tcp.cpp tcp/tcp.h tcp/tcp_socket.cpp tcp/tcp_socket.h tcp/tcp_sessions.cpp tcp/tcp_sessions.h tcp/TcpSession.cpp tcp/TcpSession.h tcp/TcpSequence.cpp tcp/TcpSequence.h tcp/tcp_in_task.cpp tcp/tcp_out_task.cpp tcp/tcp_out_segment.h tcp/states/TcpStateSynSent.cpp tcp/states/TcpStateSynSent.h tcp/states/BasicTcpState.cpp tcp/states/BasicTcpState.h)
