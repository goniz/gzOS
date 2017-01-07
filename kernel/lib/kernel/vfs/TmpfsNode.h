#ifndef GZOS_TMPFSNODE_H
#define GZOS_TMPFSNODE_H

#include "BasicVFSNode.h"

class TmpfsNode : public BasicVFSNode
{
public:
    TmpfsNode(VFSNode::Type type, std::string&& path);
    virtual ~TmpfsNode(void) = default;
};

class TmpfsDirectoryNode : public TmpfsNode
{
public:
    TmpfsDirectoryNode(VFSNode::Type type, std::string&& path);
    virtual ~TmpfsDirectoryNode() = default;

    virtual const std::vector<SharedNode>& childNodes(void) override;
    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) override;

    std::unique_ptr<FileDescriptor> open(void) override;

private:
    std::vector<SharedNode> _nodes;
};

class TmpfsFileNode : public TmpfsNode
{
public:
    TmpfsFileNode(VFSNode::Type type, std::string&& path);
    virtual ~TmpfsFileNode(void) = default;

    virtual std::unique_ptr<FileDescriptor> open(void) override;
    virtual const std::vector<SharedNode>& childNodes(void) override;
    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) override;

private:
    std::shared_ptr<std::vector<uint8_t>> _data;
};

#endif //GZOS_TMPFSNODE_H
