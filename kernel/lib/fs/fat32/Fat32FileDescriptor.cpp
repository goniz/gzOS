#include <unistd.h>
#include <fcntl.h>
#include "Fat32FileDescriptor.h"
#include "ff.h"

Fat32FileDescriptor::Fat32FileDescriptor(const char* name, int flags)
        : _filename(name),
          _fp()
{
    if (FR_OK != f_open(&_fp, name, this->translateFlags(flags))) {
        throw std::exception();
    }
}

Fat32FileDescriptor::~Fat32FileDescriptor(void) {
    this->close();
}

int Fat32FileDescriptor::read(void* buffer, size_t size) {
    unsigned int bytesRead = 0;
    int result = f_read(&_fp, buffer, size, &bytesRead);
    if (FR_OK == result) {
        return bytesRead;
    }

    return -1;
}

int Fat32FileDescriptor::write(const void* buffer, size_t size) {
#if !defined(_FS_READONLY)
    UINT bytesWritten = 0;
        int result = f_write(&_fp, buffer, size, &bytesWritten);
        if (FR_OK == result) {
            return bytesWritten;
        }
#endif
    return -1;
}

int Fat32FileDescriptor::seek(int where, int whence) {
    if (SEEK_SET != whence) {
        return -1;
    }

    int result = f_lseek(&_fp, (FSIZE_t) where);
    if (FR_OK != result) {
        return -1;
    }

    return where;
}

int Fat32FileDescriptor::stat(struct stat* stat) {
    FILINFO info;
    if (FR_OK != f_stat(_filename.c_str(), &info)) {
        return -1;
    }

    stat->st_size = info.fsize;
    return 0;
}

void Fat32FileDescriptor::close(void) {
    f_close(&_fp);
}

uint8_t Fat32FileDescriptor::translateFlags(int flags) const {
    uint8_t result = 0;

    if (flags == O_RDONLY) {
        result = FA_READ;
    } else if (flags == O_WRONLY) {
        result = FA_WRITE;
    } else if (flags == O_RDWR) {
        result = FA_READ | FA_WRITE;
    }

    if (flags & O_CREAT && flags & O_TRUNC) {
        result |= FA_CREATE_ALWAYS;
    }

    return result;
}
