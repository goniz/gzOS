#ifndef GZOS_DEVFILESYSTEM_H
#define GZOS_DEVFILESYSTEM_H

#ifdef __cplusplus
#include <lib/kernel/vfs/BasicVFSNode.h>

using FileDescriptorFactory = std::unique_ptr<FileDescriptor> (*)(void);

class DevVFSNode : public BasicVFSNode
{
public:
    DevVFSNode(std::string&& path, FileDescriptorFactory fdf);
    virtual ~DevVFSNode(void) = default;

    virtual std::unique_ptr<FileDescriptor> open(void) override;
    virtual const std::vector<SharedNode>& childNodes(void) override;
    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) override;

private:
    FileDescriptorFactory _fdFactory;
};

class DevFileSystem : public BasicVFSNode
{
public:
    enum class IoctlCommands : int {
        RegisterDevice
    };

    struct IoctlRegisterDevice {
        const char* deviceName;
        FileDescriptorFactory fdFactory;
    };

    DevFileSystem(std::string&& path);
    virtual ~DevFileSystem(void) = default;

    bool registerDevice(const char* deviceName, FileDescriptorFactory factory);

    virtual std::unique_ptr<FileDescriptor> open(void) override;
    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) override;
    virtual const std::vector<SharedNode>& childNodes(void) override;

private:
    std::vector<SharedNode> _devices;
};

class DevfsIoctlFileDescriptor : public InvalidFileDescriptor
{
public:
    DevfsIoctlFileDescriptor(DevFileSystem& devfs);
    virtual ~DevfsIoctlFileDescriptor(void) = default;

    virtual int ioctl(int cmd, void* buffer, size_t size) override;

private:
    DevFileSystem& _devfs;
};

#endif //cplusplus
#endif //GZOS_DEVFILESYSTEM_H
