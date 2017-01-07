#include "BasicVFSNode.h"

BasicVFSNode::BasicVFSNode(VFSNode::Type type, std::string&& path)
        : _type(type),
          _segment(std::move(path))
{

}

VFSNode::Type BasicVFSNode::getType(void) const {
    return _type;
}

const std::string& BasicVFSNode::getPathSegment(void) const {
    return _segment;
}

bool BasicVFSNode::mountNode(SharedNode node) {
    return false;
}

bool BasicVFSNode::isMounted(void) const {
    return false;
}
