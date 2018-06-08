#ifndef GZOS_VFSREADDIRFILEDESCRIPTOR_H
#define GZOS_VFSREADDIRFILEDESCRIPTOR_H

#include <vector>
#include <vfs/ReaddirFileDescriptor.h>
#include "VFSNode.h"

class VFSReaddirFileDescriptor : public ReaddirFileDescriptor {
public:
    VFSReaddirFileDescriptor(SharedVFSNode node);

    virtual const char* type(void) const override;

private:
    bool getNextEntry(struct DirEntry &dirEntry) override;

    SharedVFSNode _node;
    std::vector<SharedVFSNode>::const_iterator _pos;
    std::vector<SharedVFSNode>::const_iterator _end;
};

#endif //GZOS_VFSREADDIRFILEDESCRIPTOR_H
