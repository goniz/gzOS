cmake_minimum_required( VERSION 2.8 )

project(gzOS_drivers C CXX ASM)

add_subdirectory(piix4)
add_subdirectory(pcnet)
add_subdirectory(e1000)