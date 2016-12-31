project(gzOS_tmpfs C CXX ASM)

add_library(
    tmpfs STATIC
    fs_ops.cpp fs_ops.h
)