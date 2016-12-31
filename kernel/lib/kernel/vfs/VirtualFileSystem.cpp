#include <lib/primitives/hashmap.h>
#include <platform/drivers.h>
#include <platform/panic.h>
#include <lib/primitives/stringutils.h>
#include <platform/kprintf.h>
#include <deque>
#include <lib/primitives/interrupts_mutex.h>
#include "VirtualFileSystem.h"
#include "AccessControlledFileDescriptor.h"
#include "Path.h"
#include "ReaddirFileDescriptor.h"

static VirtualFileSystem* gVFS = nullptr;
static int vfs_init(void)
{
    gVFS = new VirtualFileSystem();
    return 0;
}

DECLARE_DRIVER(vfs, vfs_init, STAGE_FIRST);

VirtualFileSystem& VirtualFileSystem::instance(void) {
    auto* ptr = gVFS;
    if (nullptr == ptr) {
        panic("gVFS == nullptr\n");
    }

    return *ptr;
}

VirtualFileSystem::VirtualFileSystem(void)
    : _filesystems(),
      _supportedFs()
{

}

VirtualFileSystem::FindFsResult VirtualFileSystem::findFilesystem(const char *path) const {
    if (!path) {
        return {};
    }

    FindFsResult iterateData;

    iterateData.fs = nullptr;
    iterateData.path = path;

    _filesystems.iterate([](any_t userarg, char* key, any_t data) {
        FindFsResult* argument = (FindFsResult*) userarg;
        std::unique_ptr<FileSystem>* value = (std::unique_ptr<FileSystem>*) data;

        if (startsWith(key, argument->path)) {
            argument->fs = value->get();
            argument->path = argument->path + strlen(key) + 1;
            return MAP_FULL;
        }

        return MAP_OK;
    }, &iterateData);

    if (!iterateData.fs) {
        return {};
    }

    if (!iterateData.path) {
        return {};
    }

    return iterateData;
}

std::unique_ptr<FileDescriptor> VirtualFileSystem::open(const char *path, int flags)
{
    auto fsResult = this->findFilesystem(path);
    if (!fsResult.fs) {
        return nullptr;
    }

    auto fd = fsResult.fs->open(fsResult.path, flags);
    if (nullptr == fd) {
        return nullptr;
    }

    return std::make_unique<AccessControlledFileDescriptor>(std::move(fd), flags);
}

bool VirtualFileSystem::mountFilesystem(const char* fstype, const char* source, const char* destination) {
    if (_filesystems.get(destination)) {
        kprintf("[vfs] failed to mount %s (%s): %s is busy\n", source, fstype, destination);
        return false;
    }

    const auto fsFactory = _supportedFs.get(fstype);
    if (!fsFactory) {
        kprintf("[vfs] failed to mount %s (%s): %s is unsupported\n", source, fstype, fstype);
        return false;
    }

    std::unique_ptr<FileSystem> fs;
    try {
        fs = (*fsFactory)(source);
        if (!fs) {
            kprintf("[vfs] failed to mount %s (%s): failed to create fs\n", source, fstype, fstype);
            return false;
        }
    } catch (...) {
        kprintf("[vfs] failed to mount %s (%s): failed to create fs\n", source, fstype, fstype);
        return false;
    }

    kprintf("[vfs] successfully mounted %s of type %s on %s\n", source, fstype, destination);
    return _filesystems.put(destination, std::move(fs));
}

FileSystem* VirtualFileSystem::getFilesystem(const char *mountPoint) const {
    auto* fs = _filesystems.get(mountPoint);
    if (!fs) {
        return nullptr;
    }

    return fs->get();
}

bool VirtualFileSystem::registerFilesystem(const char* fstype, VirtualFileSystem::FilesystemFactory factory) {
    if (_supportedFs.get(fstype)) {
        return false;
    }

    return _supportedFs.put(fstype, std::move(factory));
}


class VFSReaddirFileDescriptor : public ReaddirFileDescriptor {
public:
    VFSReaddirFileDescriptor(VirtualFileSystem& vfs)
        : _vfs(vfs)
    {
        _vfs._filesystems.iterate([](any_t userarg, char * key, any_t data) -> int {
            std::deque<std::string>* fsIterations = (std::deque<std::string> *)userarg;
            fsIterations->emplace_back(key);
            return MAP_OK;
        }, &_fsIterations);
    }

private:
    bool getNextEntry(struct DirEntry &dirEntry) override {
        InterruptsMutex mutex(true);
        if (_fsIterations.empty()) {
            return false;
        }

        const auto& fsName = _fsIterations.front(); _fsIterations.pop_front();
        strncpy(dirEntry.name, fsName.c_str(), sizeof(dirEntry.name));
        dirEntry.type = DirEntryType::DIRENT_DIR;
        return true;
    }

    VirtualFileSystem& _vfs;
    std::deque<std::string> _fsIterations;
};

std::unique_ptr<FileDescriptor> VirtualFileSystem::readdir(const char* path) {
    if (path && 0 == strcmp(path, "/")) {
        return std::make_unique<AccessControlledFileDescriptor>(
                std::make_unique<VFSReaddirFileDescriptor>(*this),
                O_RDONLY
        );
    }

    auto fsResult = this->findFilesystem(path);
    if (!fsResult.fs) {
        return {};
    }

    return std::make_unique<AccessControlledFileDescriptor>(fsResult.fs->readdir(fsResult.path), O_RDONLY);
}


