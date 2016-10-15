cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_SOURCE_DIR}/../kernel/lib/syscall)

add_library(cstubs STATIC libc/exit.c libc/open.c libc/errno.c libc/close.c libc/getpid.c libc/isatty.c libc/kill.c libc/read.c libc/write.c malta/syscall.c libc/lseek.c libc/fstat.c libc/brk.c libc/start.c)