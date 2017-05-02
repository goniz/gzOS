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

    SharedVFSNode lookup(Path&& path);
    SharedVFSNode lookup(SharedVFSNode rootNode, Path&& path);

    using MountpointFactoryFunc = SharedVFSNode (*)(const char* source, const char* destName);
    bool registerFilesystem(const char* fstype, MountpointFactoryFunc factory);
    bool mountFilesystem(const char* fstype, const char* source, const char* destination);

    static VirtualFileSystem& instance(void);

private:
    SharedVFSNode createNode(const char* nodePath, VFSNode::Type type);

    SharedVFSNode _rootNode;
    HashMap<const char*, MountpointFactoryFunc> _supportedFs;
};

#endif //cplusplus
#endif //GZOS_VFS2_H
