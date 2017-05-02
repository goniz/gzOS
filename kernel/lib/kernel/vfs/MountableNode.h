#ifndef GZOS_MOUNTABLENODE_H
#define GZOS_MOUNTABLENODE_H
#ifdef __cplusplus

#include "VFSNode.h"

class MountableNode : public VFSNode
{
public:
    MountableNode(SharedVFSNode innerNode);
    virtual ~MountableNode(void) = default;

    virtual const std::string& getPathSegment(void) const override;
    virtual Type getType(void) const override;
    virtual const std::vector<SharedVFSNode>& childNodes(void) override;
    virtual SharedVFSNode createNode(VFSNode::Type type, std::string&& path) override;
    virtual size_t getSize(void) const override;

    virtual bool mountNode(SharedVFSNode node) override;
    virtual bool isMounted(void) const override;

    std::unique_ptr<FileDescriptor> open(void) override;

private:
    SharedVFSNode _innerNode;
    SharedVFSNode _mountedNode;
};

#endif //cplusplus
#endif //GZOS_MOUNTABLENODE_H
