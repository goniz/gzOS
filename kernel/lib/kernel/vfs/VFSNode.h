#ifndef GZOS_VFSNODE_H
#define GZOS_VFSNODE_H

#include <memory>
#include <vector>
#include <string>
#include <lib/kernel/vfs/FileDescriptor.h>
#include "lib/kernel/vfs/Path.h"

struct VFSNode;
using SharedVFSNode = std::shared_ptr<VFSNode>;

class VFSNode
{
public:
    enum class Type {
        File,
        Directory
    };

    virtual ~VFSNode() = default;

    virtual VFSNode::Type getType(void) const = 0;
    virtual const std::string& getPathSegment(void) const = 0;
    virtual size_t getSize(void) const = 0;

    virtual const std::vector<SharedVFSNode>& childNodes(void) = 0;
    virtual bool mountNode(SharedVFSNode node) = 0;
    virtual bool isMounted(void) const = 0;
    virtual SharedVFSNode createNode(VFSNode::Type type, std::string&& path) = 0;
    virtual std::unique_ptr<FileDescriptor> open(void) = 0;
};

#endif //GZOS_VFSNODE_H