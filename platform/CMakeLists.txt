cmake_minimum_required( VERSION 2.8 )

project(gzOS_libplatform C ASM)

include_directories("${CMAKE_CURRENT_LIST_DIR}")

add_library(platform STATIC system_init.c panic.c kprintf.c)

add_subdirectory("${CMAKE_PLATFORM_NAME}")
