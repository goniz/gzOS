#ifndef GZOS_VFSNODE_H
#define GZOS_VFSNODE_H

#include <memory>
#include <vector>
#include <string>
#include <lib/kernel/vfs/FileDescriptor.h>
#include "lib/kernel/vfs/Path.h"

struct VFSNode;
using SharedNode = std::shared_ptr<VFSNode>;

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

    virtual const std::vector<SharedNode>& childNodes(void) = 0;
    virtual bool mountNode(SharedNode node) = 0;
    virtual bool isMounted(void) const = 0;
    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) = 0;
    virtual std::unique_ptr<FileDescriptor> open(void) = 0;
};

#endif //GZOS_VFSNODE_H