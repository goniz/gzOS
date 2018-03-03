cmake_minimum_required(VERSION 2.8)

project(telnetd C CXX ASM)

add_userland_executable(telnetd
    main.c
    vlad.c
    vlad.h
    telnet_protocol.h
)