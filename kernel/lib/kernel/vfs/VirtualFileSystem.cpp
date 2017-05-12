#include <platform/kprintf.h>
#include <platform/drivers.h>
#include <platform/panic.h>
#include <vfs/AccessControlledFileDescriptor.h>
#include <proc/Scheduler.h>
#include "VirtualFileSystem.h"
#include "TmpfsNode.h"
#include "VFSReaddirFileDescriptor.h"

static VirtualFileSystem* _vfsInstance = nullptr;
static int vfs2_layer_init(void) {
    _vfsInstance = new VirtualFileSystem();
    return 0;
}

DECLARE_DRIVER(vfs2_layer, vfs2_layer_init, STAGE_FIRST);

VirtualFileSystem& VirtualFileSystem::instance(void) {
    if (nullptr == _vfsInstance) {
        panic("_vfsInstance is null");
    }

    return *_vfsInstance;
}

SharedVFSNode VirtualFileSystem::lookup(SharedVFSNode rootNode, Path&& path) {
    // let start from the baseNode provided
    SharedVFSNode node = rootNode;

    path.trim();
    path.sanitize();
    const auto pathString = std::move(path.string());

    // split the given path to its segments
    const auto segments = std::move(path.split());

    // if its "/", return the root node.
    if ("" == pathString) {
        return rootNode;
    }

    // iterate over its segments and try to walk the nodes until we find the requested node, or fail
    for (size_t i = 0; i < segments.size(); i++) {
        const auto& segment = segments[i];
        const bool is_last = (segments.size() - 1) == i;
        const bool is_first = 0 == i;

        if (is_first && "" == segment.segment) {
            continue;
        }

        // the current node can only be a non-directory if its the last segment (destination segment)
        // because if its not, we wont be able to traverse it..
        if (is_last && segment.segment == node->getPathSegment()) {
            return node;
        }

        // if its a file, and the segment is not last, we cannot find this path..
        if (VFSNode::Type::Directory != node->getType()) {
            return nullptr;
        }

        for (auto currentNode : node->childNodes()) {
            if (is_last && currentNode->getPathSegment() == segment.segment) {
                return currentNode;
            }

            if (currentNode->getPathSegment() == segment.segment &&
                VFSNode::Type::Directory == currentNode->getType()) {
                node = currentNode;
                break;
            }
        }
    }

    return nullptr;
}

SharedVFSNode VirtualFileSystem::lookup(Path&& path) {
    SharedVFSNode lookupRoot = _rootNode;
    if (!path.is_absolute()) {
        Process* currentProc = Scheduler::instance().CurrentProcess();
        if (currentProc) {
            lookupRoot = currentProc->currentWorkingNode();
        }
    }

    return this->lookup(lookupRoot, std::forward<Path>(path));
}

VirtualFileSystem::VirtualFileSystem(void)
        : _rootNode(std::make_shared<TmpfsDirectoryNode>("/"))
{

}

bool VirtualFileSystem::registerFilesystem(const char* fstype, VirtualFileSystem::MountpointFactoryFunc factory) {
    if (_supportedFs.get(fstype)) {
        return false;
    }

    return _supportedFs.put(fstype, std::move(factory));
}

bool VirtualFileSystem::mountFilesystem(const char* fstype, const char* source, const char* destination) {
    auto mountpointNode = this->lookup(Path(destination));
    if (!mountpointNode) {
        kprintf("[vfs] failed to mount %s (%s): %s does not exist\n", source, fstype, destination);
        return false;
    }

    if (_rootNode == mountpointNode || mountpointNode->isMounted()) {
        kprintf("[vfs] failed to mount %s (%s): %s is busy\n", source, fstype, destination);
        return false;
    }

    const auto fsFactory = _supportedFs.get(fstype);
    if (!fsFactory) {
        kprintf("[vfs] failed to mount %s (%s): %s is unsupported\n", source, fstype, fstype);
        return false;
    }

    SharedVFSNode newFsRootNode;
    try {
        auto dirname = Path(destination).filename();
        newFsRootNode = (*fsFactory)(source, dirname.c_str());
    } catch (...) {
        newFsRootNode = nullptr;
    }

    if (!newFsRootNode) {
        kprintf("[vfs] failed to mount %s (%s): failed to create fs\n", source, fstype, fstype);
        return false;
    }

    if (!mountpointNode->mountNode(newFsRootNode)) {
        kprintf("[vfs] failed to mount %s (%s): destination path does not allow mounting (%s)\n", source, fstype, destination);
        return false;
    }

    kprintf("[vfs] successfully mounted %s of type %s on %s\n", source, fstype, destination);
    return true;
}

std::unique_ptr<FileDescriptor> VirtualFileSystem::open(const char* path, int flags) {
    bool createdNode = false;
    auto node = this->lookup(Path(path));
    if (node && (flags & O_CREAT)) {
        return nullptr;
    }

    if (!node && (flags & O_CREAT)) {
        flags &= ~O_CREAT;
        node = this->createNode(path, VFSNode::Type::File);
        createdNode = true;
    }

    if (!node) {
        kprintf("[vfs] open: failed to find %s\n", path);
        return nullptr;
    }

    auto fd = node->open();
    if (!fd) {
        if (createdNode) {
            // TODO: remove the node we just created??
            kprintf("[vfs] failed to open fd when using O_CREAT, leaving empty file..\n");
        }

        return nullptr;
    }

    if (flags & O_TRUNC) {
        flags &= ~O_TRUNC;
        // TODO: support truncate on fds
//        fd->truncate(0);
    }

    return std::make_unique<AccessControlledFileDescriptor>(std::move(fd), flags);
}

SharedVFSNode VirtualFileSystem::createNode(const char* nodePath, VFSNode::Type type) {
    // if the node already exists, fail
    Path existingNode(nodePath);
    existingNode.trim();
    existingNode.sanitize();

    if (this->lookup(std::move(existingNode))) {
        return nullptr;
    }

    Path destinationPath(nodePath);
    auto basename = destinationPath.filename();
    if ("" == basename) {
        return nullptr;
    }

    destinationPath.trim();
    destinationPath.sanitize();
    destinationPath.up();

    auto node = this->lookup(std::move(destinationPath));
    if (!node) {
        return nullptr;
    }

    return node->createNode(type, std::move(basename));
}

std::unique_ptr<FileDescriptor> VirtualFileSystem::readdir(const char* path) {
    auto node = this->lookup(Path(path));
    if (!node) {
        return nullptr;
    }

    return std::make_unique<AccessControlledFileDescriptor>(std::make_unique<VFSReaddirFileDescriptor>(node), O_RDONLY);
}

bool VirtualFileSystem::mkdir(const char* path) {
    return nullptr != this->createNode(path, VFSNode::Type::Directory);
}

