project(gzOS_ext2 C CXX ASM)

add_library(ext2 STATIC
        ext-shell.c
        inc/blockgroup_descriptor.h
        inc/directoryentry.h
        inc/ext2access.h
        inc/inode.h
        inc/superblock.h
        inc/types.h
        ext2_fs.cpp ext2_fs.h)
