cmake_minimum_required( VERSION 2.8 )

project(gzOS_virtio_shmem C CXX ASM)

add_library(virtio_shmem STATIC virtio_shmem.cpp virtio_shmem.h)
