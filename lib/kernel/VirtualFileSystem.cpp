#include <platform/drivers.h>
#include <platform/panic.h>
#include "VirtualFileSystem.h"

static std::unique_ptr<VirtualFileSystem> global_vfs;

static int vfs_init(void)
{
    global_vfs = std::make_unique<VirtualFileSystem>();
    return 0;
}

DECLARE_DRIVER(vfs, vfs_init, STAGE_FIRST);

VirtualFileSystem *vfs(void) {
    auto* ptr = global_vfs.get();
    if (nullptr == ptr) {
        panic("global_vfs == NULL\n");
    }

    return ptr;
}

std::unique_ptr<FileDescriptor> VirtualFileSystem::open(const char *path, int flags) {
    return std::make_unique<NullFileDescriptor>();
}