#ifndef GZOS_MOUNTABLENODE_H
#define GZOS_MOUNTABLENODE_H
#ifdef __cplusplus

#include "VFSNode.h"

class MountableNode : public VFSNode
{
public:
    MountableNode(SharedNode innerNode);
    virtual ~MountableNode(void) = default;

    virtual const std::string& getPathSegment(void) const override;
    virtual Type getType(void) const override;
    virtual const std::vector<SharedNode>& childNodes(void) override;
    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) override;

    virtual bool mountNode(SharedNode node) override;
    virtual bool isMounted(void) const override;

    std::unique_ptr<FileDescriptor> open(void) override;

private:
    SharedNode _innerNode;
    SharedNode _mountedNode;
};

#endif //cplusplus
#endif //GZOS_MOUNTABLENODE_H
