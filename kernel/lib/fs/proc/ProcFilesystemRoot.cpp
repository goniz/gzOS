#include <platform/drivers.h>
#include <lib/kernel/vfs/VirtualFileSystem.h>
#include "ProcFilesystemRoot.h"

static std::shared_ptr<ProcFilesystemRoot> gProcInstance;
std::shared_ptr<ProcFilesystemRoot> ProcFilesystemRoot::instance() {
    if (nullptr == gProcInstance) {
//        gProcInstance = std::make_shared<ProcFilesystemRoot>();
    }

    return gProcInstance;
}

static int vfs_proc_init(void)
{
    VirtualFileSystem& vfs = VirtualFileSystem::instance();

    vfs.registerFilesystem("procfs", [](const char* source, const char* destName) {
        auto node = std::make_shared<ProcFilesystemRoot>(std::string(destName));
        return std::static_pointer_cast<VFSNode>(node);
    });

    return 0;
}

DECLARE_DRIVER(vfs_proc, vfs_proc_init, STAGE_SECOND);

ProcFilesystemRoot::ProcFilesystemRoot(std::string&& path)
        : BasicVFSNode(VFSNode::Type::Directory, std::move(path))
{

}

static const std::vector<SharedNode> _empty;
const std::vector<SharedNode>& ProcFilesystemRoot::childNodes(void) {
    return _empty;
}

SharedNode ProcFilesystemRoot::createNode(VFSNode::Type type, std::string&& path) {
    return nullptr;
}

std::unique_ptr<FileDescriptor> ProcFilesystemRoot::open(void) {
    return nullptr;
}
