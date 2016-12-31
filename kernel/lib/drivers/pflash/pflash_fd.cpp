#include <lib/kernel/vfs/DevFileSystem.h>
#include <platform/kprintf.h>
#include "pflash_fd.h"

PFlashFileDescriptor::PFlashFileDescriptor(uintptr_t start, uintptr_t end)
    : MemoryBackedFileDescriptor(start, end)
{

}

int PFlashFileDescriptor::write(const void* buffer, size_t size) {
    return -1;
}