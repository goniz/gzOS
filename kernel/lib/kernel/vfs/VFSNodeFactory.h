#ifndef GZOS_VFSNODEFACTORY_H
#define GZOS_VFSNODEFACTORY_H

#include "VFSNode.h"
#include "MountableNode.h"

template<typename T, typename... Args>
static inline SharedVFSNode createMountableNode(Args&& ... args) {
    auto innerNode = std::make_shared<T>(std::forward<Args>(args)...);
    return std::make_shared<MountableNode>(innerNode);
}

#endif //GZOS_VFSNODEFACTORY_H
