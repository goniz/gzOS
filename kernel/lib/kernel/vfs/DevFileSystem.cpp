#include <platform/drivers.h>
#include <lib/primitives/InterruptsMutex.h>
#include <vfs/VirtualFileSystem.h>
#include <algorithm>
#include "DevFileSystem.h"

static int vfs_dev_init(void)
{
    VirtualFileSystem& vfs = VirtualFileSystem::instance();

    vfs.registerFilesystem("devfs", [](const char* source, const char* destName) {
        auto node = std::make_shared<DevFileSystem>(std::string(destName));
        return std::static_pointer_cast<VFSNode>(node);
    });

    vfs.mkdir("/dev");
    vfs.mountFilesystem("devfs", "none", "/dev");
    return 0;
}

DECLARE_DRIVER(vfs_dev, vfs_dev_init, STAGE_SECOND);


DevVFSNode::DevVFSNode(std::string&& path, FileDescriptorFactory fdf)
    : BasicVFSNode(VFSNode::Type::File, std::move(path)),
      _fdFactory(fdf)
{

}

std::unique_ptr<FileDescriptor> DevVFSNode::open(void) {
    if (_fdFactory) {
        return _fdFactory();
    } else {
        return nullptr;
    }
}

static const std::vector<SharedVFSNode> _empty;
const std::vector<SharedVFSNode>& DevVFSNode::childNodes(void) {
    return _empty;
}

SharedVFSNode DevVFSNode::createNode(VFSNode::Type type, std::string&& path) {
    return nullptr;
}

DevFileSystem::DevFileSystem(std::string&& path)
        : BasicVFSNode(VFSNode::Type::Directory, std::move(path))
{

}

bool DevFileSystem::registerDevice(const char *deviceName, FileDescriptorFactory factory) {
    auto comparator = [deviceName](const SharedVFSNode& iter) -> bool {
        return (iter && deviceName && iter->getPathSegment() == deviceName);
    };

    auto result = std::find_if(_devices.begin(), _devices.end(), comparator);
    if (result != _devices.end()) {
        return false;
    }

    _devices.push_back(std::make_shared<DevVFSNode>(std::string(deviceName), factory));
    return true;
}

const std::vector<SharedVFSNode>& DevFileSystem::childNodes(void) {
    return _devices;
}

SharedVFSNode DevFileSystem::createNode(VFSNode::Type type, std::string&& path) {
    return nullptr;
}

std::unique_ptr<FileDescriptor> DevFileSystem::open(void) {
    return std::make_unique<DevfsIoctlFileDescriptor>(*this);
}

DevfsIoctlFileDescriptor::DevfsIoctlFileDescriptor(DevFileSystem& devfs)
        : _devfs(devfs)
{

}

int DevfsIoctlFileDescriptor::ioctl(int cmd, va_list args) {
    if ((int)DevFileSystem::IoctlCommands::RegisterDevice == cmd) {
        auto cmdbuf = va_arg(args, DevFileSystem::IoctlRegisterDevice);
//        kprintf("cmdbuf: %p\n", cmdbuf);
        kprintf("deviceName: %p fdFactory: %p\n", cmdbuf.deviceName, cmdbuf.fdFactory);
        return _devfs.registerDevice(cmdbuf.deviceName, cmdbuf.fdFactory) ? 0 : -1;
    }

    return -1;
}
