#include <platform/drivers.h>
#include "DevFileSystem.h"

static int vfs_dev_init(void)
{
    VirtualFileSystem& vfs = VirtualFileSystem::instance();

    vfs.registerFilesystem("devfs", [](const char* source) {
        return std::unique_ptr<FileSystem>(new DevFileSystem());
    });

    vfs.mountFilesystem("devfs", "none", "/dev");
    return 0;
}

DECLARE_DRIVER(vfs_dev, vfs_dev_init, STAGE_SECOND);

std::unique_ptr<FileDescriptor> DevFileSystem::open(const char* path, int flags)
{
    auto* factoryFunction = _devices.get(path);
    if (nullptr == factoryFunction || nullptr == *factoryFunction) {
        return nullptr;
    }

    return (*factoryFunction)();
}

bool DevFileSystem::registerDevice(const char *deviceName, DevFileSystem::FileDescriptorFactory factory) {
    if (_devices.get(deviceName)) {
        return false;
    }

    return _devices.put(deviceName, std::move(factory));
}

std::unique_ptr<FileDescriptor> DevFileSystem::readdir(const char* path) {
    // TODO: implement
    return nullptr;
}
