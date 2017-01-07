#include "sched/scheduler.h"
#include <vfs/VirtualFileSystem.h>
#include "vfs_api.h"

FileDescriptor* vfs_num_to_fd(int fdnum)
{
    const auto proc = Scheduler::instance().CurrentProcess();
    if (nullptr == proc) {
        return nullptr;
    }

    FileDescriptorCollection& fdc = proc->fileDescriptorCollection();
    return fdc.get_filedescriptor(fdnum);
}

static int vfs_pushfd(std::unique_ptr<FileDescriptor> fd)
{
    const auto proc = Scheduler::instance().CurrentProcess();
    if (nullptr == proc) {
        return -1;
    }

    FileDescriptorCollection& fdc = proc->fileDescriptorCollection();
    return fdc.push_filedescriptor(std::move(fd));
}

extern "C" {

int vfs_open(const char* path, int flags) {
    auto fd = VirtualFileSystem::instance().open(path, flags);
    if (nullptr == fd) {
        return -1;
    }

    return vfs_pushfd(std::move(fd));
}

int vfs_close(int fd)
{
    const auto proc = Scheduler::instance().CurrentProcess();
    if (nullptr == proc) {
        return -1;
    }

    FileDescriptorCollection& fdc = proc->fileDescriptorCollection();
    fdc.remove_filedescriptor(fd, true);
    return 0;
}

int vfs_read(int fd, void* buf, size_t size)
{
    auto fileDes = vfs_num_to_fd(fd);
    if (nullptr == fileDes) {
        return -1;
    }

    return fileDes->read(buf, size);
}

int vfs_write(int fd, const void* buf, size_t size)
{
    auto fileDes = vfs_num_to_fd(fd);
    if (nullptr == fileDes) {
        return -1;
    }

    return fileDes->write(buf, size);
}

int vfs_seek(int fd, off_t offset, int whence)
{
    auto fileDes = vfs_num_to_fd(fd);
    if (nullptr == fileDes) {
        return -1;
    }

    return fileDes->seek(offset, whence);
}

int vfs_stat(int fd, struct stat* stat)
{
    if (nullptr == stat) {
        return -1;
    }

    auto fileDes = vfs_num_to_fd(fd);
    if (nullptr == fileDes) {
        return -1;
    }

    return fileDes->stat(stat);
}

int vfs_mkdir(const char* path)
{
    return VirtualFileSystem::instance().mkdir(path);
}

int vfs_mount(const char* fstype, const char* source, const char* destination)
{
    if (VirtualFileSystem::instance().mountFilesystem(fstype, source, destination)) {
        return 0;
    }

    return -1;
}

int vfs_readdir(const char* path)
{
    auto fd = VirtualFileSystem::instance().readdir(path);
    if (!fd) {
        return -1;
    }

    return vfs_pushfd(std::move(fd));
}

} // extern "C"