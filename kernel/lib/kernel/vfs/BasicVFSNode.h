#ifndef GZOS_BASICVFSNODE_H
#define GZOS_BASICVFSNODE_H
#ifdef __cplusplus

#include "VFSNode.h"

class BasicVFSNode : public VFSNode
{
public:
    BasicVFSNode(VFSNode::Type type, std::string&& path);
    virtual ~BasicVFSNode(void) = default;

    virtual VFSNode::Type getType(void) const override;
    virtual const std::string& getPathSegment(void) const override;
    virtual bool mountNode(SharedNode node) override;
    virtual bool isMounted(void) const override;
    virtual size_t getSize(void) const override;

    virtual const std::vector<SharedNode>& childNodes(void) = 0;
    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) = 0;
    virtual std::unique_ptr<FileDescriptor> open(void) = 0;

private:
    VFSNode::Type _type;
    std::string _segment;
};

#endif //cplusplus
#endif //GZOS_BASICVFSNODE_H
