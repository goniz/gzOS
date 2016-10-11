cmake_minimum_required(VERSION 2.8)

# Mark the language as C so that CMake doesn't try to test the C++
# cross-compiler's ability to compile a simple program because that will fail
project(gzOS C CXX ASM)

SET(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin")

# enable auto include of current source directory
set(CMAKE_INCLUDE_CURRENT_DIR 1)
include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/include")
add_definitions("-Werror -Wall -Wno-error=unknown-pragmas -Wno-unknown-pragmas")

set(CMAKE_EXECUTABLE_SUFFIX ".elf")
SET(CMAKE_C_OUTPUT_EXTENSION ".o")
SET(CMAKE_CXX_OUTPUT_EXTENSION ".o")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)

add_definitions(-DTLBDEBUG=0)

add_subdirectory(platform)
add_subdirectory(stubs)
add_subdirectory(lib)
add_subdirectory(bootloader)
add_subdirectory(src)
add_subdirectory(tests)
