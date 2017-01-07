#ifndef GZOS_VFSREADDIRFILEDESCRIPTOR_H
#define GZOS_VFSREADDIRFILEDESCRIPTOR_H

#include <vector>
#include <vfs/ReaddirFileDescriptor.h>
#include "VFSNode.h"

class VFSReaddirFileDescriptor : public ReaddirFileDescriptor {
public:
    VFSReaddirFileDescriptor(SharedNode node);

private:
    bool getNextEntry(struct DirEntry &dirEntry) override;

    SharedNode _node;
    std::vector<SharedNode>::const_iterator _pos;
    std::vector<SharedNode>::const_iterator _end;
};

#endif //GZOS_VFSREADDIRFILEDESCRIPTOR_H
