#include <string>
#include <platform/drivers.h>
#include <lib/kernel/vfs/VirtualFileSystem.h>
#include <lib/kernel/proc/ProcessProvider.h>
#include "ProcFilesystemRoot.h"
#include "ProcPidNode.h"

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

const std::vector<SharedVFSNode>& ProcFilesystemRoot::childNodes(void) {
    auto& procProvider = ProcessProvider::instance();

    _nodes.clear();

    InterruptsMutex mutex(true);
    for (const auto& proc : procProvider.processList()) {
        _nodes.push_back(std::make_shared<ProcPidNode>(proc->pid()));
    }

    return _nodes;
}

SharedVFSNode ProcFilesystemRoot::createNode(VFSNode::Type type, std::string&& path) {
    return nullptr;
}

std::unique_ptr<FileDescriptor> ProcFilesystemRoot::open(void) {
    return nullptr;
}
