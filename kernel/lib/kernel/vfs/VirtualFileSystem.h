#ifndef GZOS_VFS2_H
#define GZOS_VFS2_H
#ifdef __cplusplus

#include <memory>
#include <string>
#include <lib/primitives/hashmap.h>
#include <lib/kernel/vfs/Path.h>
#include <lib/kernel/vfs/VFSNode.h>

class VirtualFileSystem
{
public:
    VirtualFileSystem(void);

    std::unique_ptr<FileDescriptor> open(const char *path, int flags);
    std::unique_ptr<FileDescriptor> readdir(const char* path);
    bool mkdir(const char* path);

    SharedNode lookup(Path&& path);
    SharedNode lookup(SharedNode rootNode, Path&& path);

    using MountpointFactoryFunc = SharedNode (*)(const char* source, const char* destName);
    bool registerFilesystem(const char* fstype, MountpointFactoryFunc factory);
    bool mountFilesystem(const char* fstype, const char* source, const char* destination);

    static VirtualFileSystem& instance(void);

private:
    SharedNode createNode(const char* nodePath, VFSNode::Type type);

    SharedNode _rootNode;
    HashMap<const char*, MountpointFactoryFunc> _supportedFs;
};

#endif //cplusplus
#endif //GZOS_VFS2_H
