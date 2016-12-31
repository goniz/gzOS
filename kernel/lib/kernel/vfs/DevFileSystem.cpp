#include <platform/drivers.h>
#include <lib/primitives/interrupts_mutex.h>
#include <deque>
#include "DevFileSystem.h"
#include "ReaddirFileDescriptor.h"

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

class DevReaddirFileDescriptor : public ReaddirFileDescriptor {
public:
    DevReaddirFileDescriptor(const DevFileSystem& fs)
            : _fs(fs)
    {
        _fs._devices.iterate([](any_t userarg, char * key, any_t data) -> int {
            std::deque<std::string>* fsIterations = (std::deque<std::string> *)userarg;
            fsIterations->emplace_back(key);
            return MAP_OK;
        }, &_devices);
    }

private:
    bool getNextEntry(struct DirEntry &dirEntry) override {
        InterruptsMutex mutex(true);
        if (_devices.empty()) {
            return false;
        }

        const auto& devName = _devices.front(); _devices.pop_front();
        strncpy(dirEntry.name, devName.c_str(), sizeof(dirEntry.name));
        dirEntry.type = DirEntryType::DIRENT_REG;
        return true;
    }

    const DevFileSystem& _fs;
    std::deque<std::string> _devices;
};

std::unique_ptr<FileDescriptor> DevFileSystem::readdir(const char* path) {
    return std::make_unique<DevReaddirFileDescriptor>(*this);
}
