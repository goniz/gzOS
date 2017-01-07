#include "MountableNode.h"

MountableNode::MountableNode(SharedNode innerNode)
    : _innerNode(innerNode)
{

}

const std::vector<SharedNode>& MountableNode::childNodes(void) {
    if (_mountedNode) {
        return _mountedNode->childNodes();
    } else {
        return _innerNode->childNodes();
    }
}

bool MountableNode::mountNode(SharedNode node) {
    _mountedNode = node;
    return true;
}

SharedNode MountableNode::createNode(VFSNode::Type type, std::string&& path) {
    if (_mountedNode) {
        return _mountedNode->createNode(type, std::move(path));
    } else {
        return _innerNode->createNode(type, std::move(path));
    }
}

VFSNode::Type MountableNode::getType(void) const {
    if (_mountedNode) {
        return _mountedNode->getType();
    } else {
        return _innerNode->getType();
    }
}

const std::string& MountableNode::getPathSegment(void) const {
    if (_mountedNode) {
        return _mountedNode->getPathSegment();
    } else {
        return _innerNode->getPathSegment();
    }
}

std::unique_ptr<FileDescriptor> MountableNode::open(void) {
    if (_mountedNode) {
        return _mountedNode->open();
    } else {
        return _innerNode->open();
    }
}

bool MountableNode::isMounted(void) const {
    if (_mountedNode) {
        return true;
    } else {
        return _innerNode->isMounted();
    }
}
