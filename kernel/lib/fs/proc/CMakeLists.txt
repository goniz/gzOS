project(gzOS_procfs C CXX ASM)

add_library(
    procfs STATIC
    ProcFilesystemRoot.cpp ProcFilesystemRoot.h
)