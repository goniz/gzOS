cmake_minimum_required( VERSION 2.8 )

project(gzOS_libplatform C CXX ASM)

include_directories("${CMAKE_CURRENT_LIST_DIR}")

add_library(platform STATIC system_init.c panic.c kprintf.c sbrk.cpp pci/pci.h pci/pci.cpp pci/pci_ids.c pci/pci_drivers.h drivers.h drivers.cpp)
add_definitions("-DPLATFORM=${CMAKE_PLATFORM_NAME}")
add_subdirectory("${CMAKE_PLATFORM_NAME}")
