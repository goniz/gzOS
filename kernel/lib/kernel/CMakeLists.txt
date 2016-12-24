cmake_minimum_required( VERSION 2.8 )

project(gzOS_libkernel C CXX ASM)

add_library(kernel STATIC sched/scheduler.cpp proccess/process.h proccess/process.cpp signals.h syscalls.cpp c_api.cpp vfs/FileDescriptorCollection.cpp vfs/FileDescriptorCollection.h vfs/FileDescriptor.cpp vfs/FileDescriptor.h vfs/VirtualFileSystem.cpp vfs/VirtualFileSystem.h elf/elf.h sched/thread.cpp sched/thread.h IdAllocator.cpp IdAllocator.h proccess/ProcessMemoryMap.cpp proccess/ProcessMemoryMap.h elf/ElfLoader.cpp elf/ElfLoader.h vfs/ConsoleFileDescriptor.cpp vfs/ConsoleFileDescriptor.h vfs/DevFileSystem.cpp vfs/DevFileSystem.h vfs/AccessControlledFileDescriptor.cpp vfs/AccessControlledFileDescriptor.h vfs/Path.h vfs/FileSystem.h vfs/ReaddirFileDescriptor.h vfs/ReaddirFileDescriptor.cpp)