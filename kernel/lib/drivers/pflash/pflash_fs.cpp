#include <platform/drivers.h>
#include <lib/kernel/vfs/DevFileSystem.h>
#include <lib/kernel/vfs/FileSystem.h>
#include <lib/kernel/vfs/ReaddirFileDescriptor.h>
#include <lib/primitives/interrupts_mutex.h>
#include <lib/primitives/array.h>
#include "pflash_fs.h"
#include "pflash_fd.h"

static int dev_pflash_init(void)
{
    VirtualFileSystem& vfs = VirtualFileSystem::instance();

    vfs.registerFilesystem("pflashfs", [](const char* source) {
        return std::unique_ptr<FileSystem>(new PFlashFileSystem());
    });

    vfs.mountFilesystem("pflashfs", "none", "/pflash");
    return 0;
}

DECLARE_DRIVER(dev_flash, dev_pflash_init, STAGE_SECOND + 1);

PFlashFileSystem::PFlashFileSystem(void)
{

}

std::unique_ptr<FileDescriptor> PFlashFileSystem::open(const char *path, int flags) {
    for (const auto part : _partitions) {
        int ret = strcmp(part.name, path);
        if (0 != ret) {
            continue;
        }

        auto start = _flash_base + part.offset;
        auto end = start + part.size;
        return std::make_unique<PFlashFileDescriptor>(start, end);
    }

    return nullptr;
}

class PFlashReaddirFileDescriptor : public ReaddirFileDescriptor {
public:
    PFlashReaddirFileDescriptor(PFlashFileSystem& fs)
        : _index(0),
          _fs(fs)
    {

    }

private:
    bool getNextEntry(struct DirEntry &dirEntry) override {
        InterruptsMutex mutex(true);
        if ((ARRAY_SIZE(_fs._partitions) - 1) < (uint32_t)_index) {
            return false;
        }

        strncpy(dirEntry.name, _fs._partitions[_index].name, sizeof(dirEntry.name));

        _index++;
        return true;
    }

    int _index;
    PFlashFileSystem& _fs;
};

std::unique_ptr<FileDescriptor> PFlashFileSystem::readdir(const char* path) {
    return std::make_unique<PFlashReaddirFileDescriptor>(*this);
}
