#ifndef GZOS_EXT2_FS_H
#define GZOS_EXT2_FS_H

#include <lib/kernel/vfs/VirtualFileSystem.h>
#include "ff.h"

#ifdef __cplusplus

class Fat32FileSystem : public FileSystem
{
    friend class Fat32ReaddirFileDescriptor;

public:
    Fat32FileSystem(const char* sourceDevice);
    virtual std::unique_ptr<FileDescriptor> open(const char *path, int flags) override;

    std::unique_ptr<FileDescriptor> readdir(const char* path) override;

private:
    std::unique_ptr<FileDescriptor> _sourceFd;
    FATFS _fs;
};

#endif //cplusplus
#endif //GZOS_EXT2_FS_H
