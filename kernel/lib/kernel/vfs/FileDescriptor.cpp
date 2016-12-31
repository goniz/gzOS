#include <cstring>
#include <cstdio>
#include <algorithm>
#include "FileDescriptor.h"

int NullFileDescriptor::read(void *buffer, size_t size) {
    memset(buffer, 0, size);
    this->offset += size;
    return (int) size;
}

int NullFileDescriptor::write(const void *buffer, size_t size) {
    (void)buffer;
    this->offset += size;
    return (int) size;
}

int NullFileDescriptor::seek(int where, int whence) {
    switch (whence) {
        case SEEK_SET:
            this->offset = where;
            break;
        case SEEK_CUR:
            this->offset += where;
            break;
        // fall through
        case SEEK_END:
        default:
            return -1;
    }

    return this->offset;
}

void NullFileDescriptor::close(void) {
    this->offset = 0;
}

int NullFileDescriptor::stat(struct stat *stat) {
    stat->st_size = 0;
    return 0;
}

int InvalidFileDescriptor::read(void *buffer, size_t size) {
    return -1;
}

int InvalidFileDescriptor::write(const void *buffer, size_t size) {
    return -1;
}

int InvalidFileDescriptor::seek(int where, int whence) {
    return -1;
}

void InvalidFileDescriptor::close(void) {

}

int InvalidFileDescriptor::stat(struct stat *stat) {
    return -1;
}

int FileDescriptor::get_offset(void) const {
    return this->offset;
}

bool FileDescriptor::is_valid(void) const {
    return nullptr != dynamic_cast<const InvalidFileDescriptor*>(this);
}

MemoryBackedFileDescriptor::MemoryBackedFileDescriptor(uintptr_t start, uintptr_t end)
        : _start(start),
          _end(end)
{

}

int MemoryBackedFileDescriptor::read(void *buffer, size_t size) {
    const auto length = std::min(size, this->size_left());
    if (0 == length) {
        return 0;
    }

    memcpy(buffer, (const void *) (_start + this->offset), length);
    this->offset += length;
    return (int) length;
}

int MemoryBackedFileDescriptor::write(const void *buffer, size_t size) {
    const auto length = std::min(size, this->size_left());
    if (0 == length) {
        return 0;
    }

    memcpy((void*) (_start + this->offset), buffer, length);
    this->offset += length;
    return (int) length;
}

int MemoryBackedFileDescriptor::seek(int where, int whence) {
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

void MemoryBackedFileDescriptor::close(void) {

}

int MemoryBackedFileDescriptor::stat(struct stat *stat) {
    stat->st_size = _end - _start;
    return 0;
}
