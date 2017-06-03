#ifndef GZOS_PROCFILESYSTEMROOT_H
#define GZOS_PROCFILESYSTEMROOT_H
#ifdef __cplusplus

#include <lib/kernel/vfs/BasicVFSNode.h>
#include <lib/primitives/SuspendableMutex.h>

class ProcFilesystemRoot : public BasicVFSNode
{
public:
    ProcFilesystemRoot(std::string&& path);
    virtual ~ProcFilesystemRoot(void) = default;

    virtual const std::vector<SharedVFSNode>& childNodes(void) override;
    virtual SharedVFSNode createNode(VFSNode::Type type, std::string&& path) override;
    virtual std::unique_ptr<FileDescriptor> open(void) override;

private:
    std::vector<SharedVFSNode> _nodes;
    SuspendableMutex _nodesMutex;
};

#endif //cplusplus
#endif //GZOS_PROCFILESYSTEMROOT_H
