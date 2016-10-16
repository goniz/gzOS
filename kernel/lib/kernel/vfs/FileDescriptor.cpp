#include <cstring>
#include <cstdio>
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

int FileDescriptor::get_offset(void) const {
    return this->offset;
}

bool FileDescriptor::is_valid(void) const {
    return nullptr != dynamic_cast<const InvalidFileDescriptor*>(this);
}
