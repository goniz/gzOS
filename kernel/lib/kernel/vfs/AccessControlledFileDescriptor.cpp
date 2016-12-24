#include <fcntl.h>
#include "AccessControlledFileDescriptor.h"

AccessControlledFileDescriptor::AccessControlledFileDescriptor(std::unique_ptr<FileDescriptor> fd, int flags)
    : _fd(std::move(fd)),
      _writable(false),
      _readable(false)
{
    if (O_RDONLY == flags) {
        _readable = true;
        _writable = false;
    }

    if (O_WRONLY == flags) {
        _readable = false;
        _writable = true;
    }

    if (O_RDWR == flags) {
        _readable = true;
        _writable = true;
    }
}

int AccessControlledFileDescriptor::read(void *buffer, size_t size) {
    if (!_readable) {
        return -1;
    }

    return _fd->read(buffer, size);
}

int AccessControlledFileDescriptor::write(const void *buffer, size_t size) {
    if (!_writable) {
        return -1;
    }

    return _fd->write(buffer, size);
}

int AccessControlledFileDescriptor::seek(int where, int whence) {
    return _fd->seek(where, whence);
}

void AccessControlledFileDescriptor::close(void) {
    _fd->close();
}
