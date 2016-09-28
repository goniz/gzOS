#include <lib/primitives/lock_guard.h>
#include "FileDescriptorCollection.h"

int FileDescriptorCollection::push_filedescriptor(std::unique_ptr<FileDescriptor> fd) {
    int fdnum = this->allocate_fd();
    if (INVALID_FD == fdnum) {
        return INVALID_FD;
    }

    _mutex.lock();
    _fds[fdnum] = std::move(fd);
    _mutex.unlock();

    return fdnum;
}

int FileDescriptorCollection::allocate_fd(void) {
    lock_guard<InterruptsMutex> guard(_mutex);

    int new_fd = 0;
    for (; new_fd < MAX_FD; new_fd++) {
        if (_fds.end() == _fds.find(new_fd)) {
            return new_fd;
        }
    }

    return INVALID_FD;
}

FileDescriptor* FileDescriptorCollection::get_filedescriptor(int fdnum) {
    _mutex.lock();
    auto fd = _fds.find(fdnum);
    _mutex.unlock();

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

    if (close) {
        fd->close();
    }

    _mutex.lock();
    _fds.erase(fdnum);
    _mutex.unlock();

    return 0;
}

void FileDescriptorCollection::close_all(void) {
    lock_guard<InterruptsMutex> guard(_mutex);

    for (const auto& iter : _fds) {
        const auto& fd = iter.second;
        if (nullptr != fd) {
            fd->close();
        }
    }

    _fds.clear();
}
