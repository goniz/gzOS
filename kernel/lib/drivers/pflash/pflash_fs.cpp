#include <platform/drivers.h>
#include <lib/kernel/vfs/DevFileSystem.h>
#include <lib/kernel/vfs/ReaddirFileDescriptor.h>
#include <lib/primitives/interrupts_mutex.h>
#include <lib/primitives/array.h>
#include <lib/kernel/vfs/VirtualFileSystem.h>
#include "pflash_fs.h"
#include "pflash_fd.h"

static int dev_pflash_init(void) {
    VirtualFileSystem& vfs = VirtualFileSystem::instance();


    vfs.registerFilesystem("pflashfs", [](const char* source, const char* destName) -> SharedVFSNode {
        return std::static_pointer_cast<VFSNode>(std::make_shared<PFlashFileSystem>(std::string(destName)));
    });

    return 0;
}

DECLARE_DRIVER(dev_flash, dev_pflash_init, STAGE_SECOND + 1);

PFlashFileSystem::PFlashFileSystem(std::string&& path)
        : BasicVFSNode(VFSNode::Type::Directory, std::move(path))
{
    for (const auto& part : _partitions) {
        auto node = std::make_shared<PFlashVFSNode>(part);
        _nodes.push_back(std::static_pointer_cast<VFSNode>(node));
    }
}

std::unique_ptr<FileDescriptor> PFlashFileSystem::open(void) {
    return nullptr;
}

const std::vector<SharedVFSNode>& PFlashFileSystem::childNodes(void) {
    return _nodes;
}

SharedVFSNode PFlashFileSystem::createNode(VFSNode::Type type, std::string&& path) {
    return nullptr;
}

PFlashVFSNode::PFlashVFSNode(const PFlashPartition& partition)
    : BasicVFSNode(VFSNode::Type::File, std::string(partition.name)),
      _partition(partition)
{

}

std::unique_ptr<FileDescriptor> PFlashVFSNode::open(void) {
    auto start = _flash_base + _partition.offset;
    auto end = start + _partition.size;
    return std::make_unique<PFlashFileDescriptor>(start, end);
}

static std::vector<SharedVFSNode> _empty;
const std::vector<SharedVFSNode>& PFlashVFSNode::childNodes(void) {
    return _empty;
}

SharedVFSNode PFlashVFSNode::createNode(VFSNode::Type type, std::string&& path) {
    return nullptr;
}

size_t PFlashVFSNode::getSize(void) const {
    return _partition.size;
}
