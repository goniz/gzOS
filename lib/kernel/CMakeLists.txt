cmake_minimum_required( VERSION 2.8 )

project(gzOS_libkernel C CXX ASM)

add_library(kernel STATIC scheduler.cpp process.h process.cpp signals.h syscalls.cpp c_api.cpp FileDescriptorCollection.cpp FileDescriptorCollection.h FileDescriptor.cpp FileDescriptor.h VirtualFileSystem.cpp VirtualFileSystem.h)
