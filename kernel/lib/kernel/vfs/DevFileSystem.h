#ifndef GZOS_DEVFILESYSTEM_H
#define GZOS_DEVFILESYSTEM_H

#ifdef __cplusplus
#include "VirtualFileSystem.h"
#include <lib/kernel/vfs/FileSystem.h>

class DevFileSystem : public FileSystem
{
    friend class DevReaddirFileDescriptor;

public:
    using FileDescriptorFactory = std::unique_ptr<FileDescriptor> (*)(void);

    bool registerDevice(const char* deviceName, FileDescriptorFactory factory);

    virtual std::unique_ptr<FileDescriptor> open(const char *path, int flags) override;

    std::unique_ptr<FileDescriptor> readdir(const char* path) override;

private:
    HashMap<const char*, FileDescriptorFactory> _devices;
};

#endif //cplusplus
#endif //GZOS_DEVFILESYSTEM_H
