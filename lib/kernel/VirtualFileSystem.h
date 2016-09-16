#ifndef GZOS_VIRTUALFILESYSTEM_H
#define GZOS_VIRTUALFILESYSTEM_H

#ifdef __cplusplus
#include <memory>
#include <map>
#include <sys/types.h>
#include "FileDescriptor.h"

class FileSystem {

};

class VirtualFileSystem {
public:
    std::unique_ptr<FileDescriptor> open(const char* path, int flags);

private:
    std::map<std::string, std::unique_ptr<FileSystem>> _filesystems;
};

VirtualFileSystem* vfs(void);
#endif // cplusplus

#ifdef __cplusplus
extern "C" {
#endif // cplusplus

int vfs_open(const char *path, int flags);
int vfs_close(int fd);
int vfs_read(int fd, void* buf, size_t size);
int vfs_write(int fd, const void* buf, size_t size);
int vfs_seek(int fd, off_t offset, int whence);

#ifdef __cplusplus
}
#endif // cplusplus
#endif //GZOS_VIRTUALFILESYSTEM_H
