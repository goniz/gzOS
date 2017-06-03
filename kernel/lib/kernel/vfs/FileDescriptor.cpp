#include <cstring>
#include <cstdio>
#include <algorithm>
#include <lib/primitives/align.h>
#include "FileDescriptor.h"

int NullFileDescriptor::read(void* buffer, size_t size) {
    memset(buffer, 0, size);
    this->offset += size;
    return (int) size;
}

int NullFileDescriptor::write(const void* buffer, size_t size) {
    (void) buffer;
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

int NullFileDescriptor::stat(struct stat* stat) {
    stat->st_size = 0;
    return 0;
}

int InvalidFileDescriptor::read(void* buffer, size_t size) {
    return -1;
}

int InvalidFileDescriptor::write(const void* buffer, size_t size) {
    return -1;
}

int InvalidFileDescriptor::seek(int where, int whence) {
    return -1;
}

void InvalidFileDescriptor::close(void) {

}

int InvalidFileDescriptor::stat(struct stat* stat) {
    return -1;
}

int FileDescriptor::get_offset(void) const {
    return this->offset;
}

bool FileDescriptor::is_valid(void) const {
    return nullptr != dynamic_cast<const InvalidFileDescriptor*>(this);
}

int FileDescriptor::ioctl(int cmd, void* buffer, size_t size) {
    return -1;
}

MemoryBackedFileDescriptor::MemoryBackedFileDescriptor(uintptr_t start, uintptr_t end)
        : _start(start),
          _end(end) {

}

int MemoryBackedFileDescriptor::read(void* buffer, size_t size) {
    const auto length = std::min(size, this->size_left());
    if (0 == length) {
        return 0;
    }

    memcpy(buffer, (const void*) (_start + this->offset), length);
    this->offset += length;
    return (int) length;
}

int MemoryBackedFileDescriptor::write(const void* buffer, size_t size) {
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

int MemoryBackedFileDescriptor::stat(struct stat* stat) {
    stat->st_size = _end - _start;
    return 0;
}

VectorBackedFileDescriptor::VectorBackedFileDescriptor(std::shared_ptr<std::vector<uint8_t>> vector)
        : _data(vector) {

}

size_t VectorBackedFileDescriptor::size_left(void) {
    return _data->size() - this->offset;
}

int VectorBackedFileDescriptor::read(void* buffer, size_t size) {
    const auto length = std::min(size, this->size_left());
    if (0 == length) {
        return 0;
    }

    memcpy(buffer, (const void*) (_data->data() + this->offset), length);
    this->offset += length;
    return (int) length;
}

int VectorBackedFileDescriptor::write(const void* buffer, size_t size) {
    const uint8_t* start = (const uint8_t*) buffer;
    const uint8_t* end = start + size;

    _data->insert(_data->end(), start, end);

    this->offset += size;
    return (int) size;
}

int VectorBackedFileDescriptor::seek(int where, int whence) {
    const auto start = _data->begin().base();
    const auto end = _data->end().base();
    switch (whence) {
        case SEEK_SET:
            if (!pointer_is_in_range(where, start, end)) {
                return -1;
            }

            this->offset = where;
            break;

        case SEEK_CUR:
            if (!pointer_is_in_range(this->offset + where, start, end)) {
                return -1;
            }

            this->offset += where;
            break;

        case SEEK_END:
            if (!pointer_is_in_range(_data->size() - where, start, end)) {
                return -1;
            }

            this->offset = (int) (_data->size() - where);
            break;

        default:
            return -1;
    }

    return this->offset;
}

void VectorBackedFileDescriptor::close(void) {

}

int VectorBackedFileDescriptor::stat(struct stat* stat) {
    stat->st_size = _data->size();
    return 0;
}

DuplicatedFileDescriptor::DuplicatedFileDescriptor(std::unique_ptr<FileDescriptor>& dupFd)
    : _dupFd(dupFd)
{

}

int DuplicatedFileDescriptor::read(void* buffer, size_t size) {
    if (_dupFd) {
        return _dupFd->read(buffer, size);
    } else {
        return -1;
    }
}

int DuplicatedFileDescriptor::write(const void* buffer, size_t size) {
    if (_dupFd) {
        return _dupFd->write(buffer, size);
    } else {
        return -1;
    }
}

int DuplicatedFileDescriptor::seek(int where, int whence) {
    if (_dupFd) {
        return _dupFd->seek(where, whence);
    } else {
        return -1;
    }
}

int DuplicatedFileDescriptor::stat(struct stat* stat) {
    if (_dupFd) {
        return _dupFd->stat(stat);
    } else {
        return -1;
    }
}

int DuplicatedFileDescriptor::poll(bool* read_ready, bool* write_ready) {
    if (_dupFd) {
        return _dupFd->poll(read_ready, write_ready);
    } else {
        return -1;
    }
}

void DuplicatedFileDescriptor::close(void) {
    if (_dupFd) {
        _dupFd->close();
    }
}
