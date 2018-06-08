#include <lib/primitives/LockGuard.h>
#include "FileDescriptorCollection.h"

FileDescriptorCollection::FileDescriptorCollection(const FileDescriptorCollection& other)
    : _fds(other._fds)
{

}

int FileDescriptorCollection::push_filedescriptor(std::unique_ptr<FileDescriptor> fd) {
    if (nullptr == fd) {
        return INVALID_FD;
    }

    InterruptsMutex mutex(true);

    int fdnum = this->allocate_fd();
    if (INVALID_FD == fdnum) {
        return INVALID_FD;
    }

    _fds[fdnum] = std::shared_ptr<FileDescriptor>(std::move(fd));
    return fdnum;
}

int FileDescriptorCollection::allocate_fd(void) {
    InterruptsMutex mutex(true);

    int new_fd = 0;
    for (; new_fd < MAX_FD; new_fd++) {
        if (_fds.end() == _fds.find(new_fd)) {
            return new_fd;
        }
    }

    return INVALID_FD;
}

FileDescriptor* FileDescriptorCollection::get_filedescriptor(int fdnum) {
    InterruptsMutex mutex(true);

    auto fd = _fds.find(fdnum);
    if (_fds.end() == fd) {
        return nullptr;
    }

    return fd->second.get();
}

int FileDescriptorCollection::remove_filedescriptor(int fdnum, bool close) {
    auto* fd = this->get_filedescriptor(fdnum);
    if (nullptr == fd) {
        return -1;
    }

#if 0
    kprintf("remove_fd(%d) --> %s\n", fdnum, fd->type());
#endif

    if (close) {
        try { fd->close(); } catch (...) { }
    }

    InterruptsMutex mutex(true);
    try { _fds.erase(fdnum); } catch (...) { }

    return 0;
}

void FileDescriptorCollection::close_all(void) {
    InterruptsMutex mutex(true);

    _fds.clear();
}

bool FileDescriptorCollection::duplicate(int old_fd, int new_fd) {
    InterruptsMutex mutex(true);

    auto oldFileDesc = _fds.find(old_fd);
    if (_fds.end() == oldFileDesc) {
        return false;
    }

    _fds[new_fd] = oldFileDesc->second;
    return true;
}

void FileDescriptorCollection::dump_fds(void) const {
    for (auto& fd : _fds) {
        kprintf("fd %d type %s\n", fd.first, fd.second->type());
    }
}
