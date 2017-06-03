#include <lib/primitives/InterruptsMutex.h>
#include "VFSReaddirFileDescriptor.h"
#include "VFSNode.h"

VFSReaddirFileDescriptor::VFSReaddirFileDescriptor(SharedVFSNode node)
        : _node(node),
          _pos(_node->childNodes().begin()),
          _end(_node->childNodes().end())
{

}

bool VFSReaddirFileDescriptor::getNextEntry(struct DirEntry& dirEntry) {
    InterruptsMutex mutex(true);
    if (_pos >= _end) {
        return false;
    }

    auto node = *_pos++;
    if (!node) {
        return false;
    }

    strncpy(dirEntry.name, node->getPathSegment().c_str(), sizeof(dirEntry.name));
    dirEntry.type = node->getType() == VFSNode::Type::Directory ? DIRENT_DIR : DIRENT_REG;
    dirEntry.size = node->getSize();
    return true;
}
