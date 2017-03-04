cmake_minimum_required(VERSION 2.8)

add_definitions(-s -g0 -Os)

add_subdirectory(helloworld)
add_subdirectory(udp_echo)
add_subdirectory(duktape)
add_subdirectory(pflash)
add_subdirectory(ls)
add_subdirectory(shell)
add_subdirectory(nc)
add_subdirectory(pull_file)