#ifndef GZOS_VFS_API_H
#define GZOS_VFS_API_H

#include <lib/primitives/hashmap.h>
#include "FileDescriptor.h"

#ifdef __cplusplus
FileDescriptor* vfs_num_to_fd(int fdnum);
#endif

#ifdef __cplusplus
extern "C" {
#endif

int vfs_open(const char *path, int flags);
int vfs_close(int fd);
int vfs_read(int fd, void* buf, size_t size);
int vfs_write(int fd, const void* buf, size_t size);
int vfs_seek(int fd, off_t offset, int whence);
int vfs_dup(int old_fd, int new_fd);
int vfs_stat(int fd, struct stat* stat);
int vfs_mkdir(const char* path);
int vfs_mount(const char* fstype, const char* source, const char* destination);
int vfs_readdir(const char* path);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_VFS_API_H
