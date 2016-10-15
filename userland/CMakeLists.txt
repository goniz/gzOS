cmake_minimum_required(VERSION 2.8)

project(userland C CXX ASM)

add_definitions(-D__USERSPACE=1)

include(toolchain/add_userland_executable.cmake)
add_subdirectory(toolchain)
add_subdirectory(apps)