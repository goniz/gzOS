#include <lib/kernel/vfs/DevFileSystem.h>
#include <platform/kprintf.h>
#include "pflash_fd.h"

PFlashFileDescriptor::PFlashFileDescriptor(uintptr_t start, uintptr_t end)
    : _start(start),
      _end(end)
{

}

int PFlashFileDescriptor::read(void* buffer, size_t size) {
    const auto length = std::min(size, this->size_left());
    if (0 == length) {
        return 0;
    }

    memcpy(buffer, (const void *) (_start + this->offset), length);
    this->offset += length;
    return (int) length;
}

int PFlashFileDescriptor::write(const void* buffer, size_t size) {
    return -1;
}

int PFlashFileDescriptor::seek(int where, int whence) {
    switch (whence) {
        case SEEK_SET:
            this->offset = where;
            break;

        case SEEK_CUR:
            this->offset += where;
            break;

        case SEEK_END:
            this->offset = (int) ((_end - where) - _start);
            break;

        default:
            return -1;
    }

    return this->offset;
}

void PFlashFileDescriptor::close(void) {

}
