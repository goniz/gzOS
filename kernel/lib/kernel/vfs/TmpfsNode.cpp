#include "TmpfsNode.h"
#include "VFSNodeFactory.h"

TmpfsNode::TmpfsNode(VFSNode::Type type, std::string&& path)
        : BasicVFSNode(type, std::move(path))
{

}

const std::vector<SharedNode>& TmpfsDirectoryNode::childNodes(void) {
    return _nodes;
}

SharedNode TmpfsDirectoryNode::createNode(VFSNode::Type type, std::string&& path) {
    std::shared_ptr<TmpfsNode> newNode;

    if (VFSNode::Type::Directory == type) {
        newNode = std::static_pointer_cast<TmpfsNode>(createMountableNode<TmpfsDirectoryNode>(type, std::move(path)));
    } else {
        newNode = std::static_pointer_cast<TmpfsNode>(createMountableNode<TmpfsFileNode>(type, std::move(path)));
    }

    _nodes.push_back(newNode);
    return newNode;
}

std::unique_ptr<FileDescriptor> TmpfsFileNode::open(void) {
    return std::make_unique<VectorBackedFileDescriptor>(_data);
}

TmpfsFileNode::TmpfsFileNode(VFSNode::Type type, std::string&& path)
        : TmpfsNode(type, std::move(path)),
          _data(std::make_shared<std::vector<uint8_t>>())
{
    _data->reserve(1024);
}

static std::vector<SharedNode> _empty{};
const std::vector<SharedNode>& TmpfsFileNode::childNodes(void) {
    return _empty;
}

SharedNode TmpfsFileNode::createNode(VFSNode::Type type, std::string&& path) {
    return nullptr;
}

TmpfsDirectoryNode::TmpfsDirectoryNode(VFSNode::Type type, std::string&& path)
        : TmpfsNode(type, std::move(path))
{

}

std::unique_ptr<FileDescriptor> TmpfsDirectoryNode::open(void) {
    return nullptr;
}
