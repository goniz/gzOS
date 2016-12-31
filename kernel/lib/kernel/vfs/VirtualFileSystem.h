#ifndef GZOS_VIRTUALFILESYSTEM_H
#define GZOS_VIRTUALFILESYSTEM_H

#ifdef __cplusplus
#include <memory>
#include <sys/types.h>
#include <lib/primitives/hashmap.h>
#include "FileDescriptor.h"
#include "Path.h"
#include "FileSystem.h"

class VirtualFileSystem : public FileSystem {
    friend class VFSReaddirFileDescriptor;
public:
    using FilesystemFactory = std::unique_ptr<FileSystem> (*)(const char* source);

    VirtualFileSystem(void);

    virtual std::unique_ptr<FileDescriptor> open(const char* path, int flags) override;
    virtual std::unique_ptr<FileDescriptor> readdir(const char* path) override;

    bool mountFilesystem(const char* fstype, const char* source, const char* destination);

    bool registerFilesystem(const char* fstype, FilesystemFactory factory);
    FileSystem* getFilesystem(const char* mountPoint) const;

    static VirtualFileSystem& instance(void);

private:
    struct FindFsResult {
        FileSystem* fs;
        const char* path;
    };

    FindFsResult findFilesystem(const char *path) const;

    HashMap<Path, std::unique_ptr<FileSystem>> _filesystems;
    HashMap<const char*, FilesystemFactory> _supportedFs;
};

template<>
struct StringKeyConverter<Path> {
    static void generate_key(const Path& key, char* output_key, size_t size) {
        StringKeyConverter<const char*>::generate_key(key.string().c_str(), output_key, size);
    }
};

#endif // cplusplus

#ifdef __cplusplus
extern "C" {
#endif // cplusplus

FileDescriptor* vfs_num_to_fd(int fdnum);
int vfs_open(const char *path, int flags);
int vfs_close(int fd);
int vfs_read(int fd, void* buf, size_t size);
int vfs_write(int fd, const void* buf, size_t size);
int vfs_seek(int fd, off_t offset, int whence);
int vfs_stat(int fd, struct stat* stat);
int vfs_mount(const char* fstype, const char* source, const char* destination);
int vfs_readdir(const char* path);

#ifdef __cplusplus
}
#endif // cplusplus
#endif //GZOS_VIRTUALFILESYSTEM_H
