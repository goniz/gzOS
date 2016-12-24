cmake_minimum_required( VERSION 2.8 )

project(gzOS_qemu_shmem C CXX ASM)

add_library(qemu_shmem STATIC qemu_shmem.cpp qemu_shmem.h)
