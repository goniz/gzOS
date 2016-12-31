project(gzOS_fat32 C CXX ASM)

add_library(
    fat32 STATIC
    diskio.cpp diskio.h
    ff.c ff.h
    ffconf.h
    integer.h
    fs_ops.cpp fs_ops.h
)