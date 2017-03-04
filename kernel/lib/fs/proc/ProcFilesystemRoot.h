#ifndef GZOS_PROCFILESYSTEMROOT_H
#define GZOS_PROCFILESYSTEMROOT_H
#ifdef __cplusplus

#include <lib/kernel/vfs/BasicVFSNode.h>

class ProcFilesystemRoot : public BasicVFSNode
{
public:
    ProcFilesystemRoot(std::string&& path);
    virtual ~ProcFilesystemRoot(void) = default;

    virtual const std::vector<SharedNode>& childNodes(void) override;
    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) override;
    virtual std::unique_ptr<FileDescriptor> open(void) override;

    static std::shared_ptr<ProcFilesystemRoot> instance();
};

#endif //cplusplus
#endif //GZOS_PROCFILESYSTEMROOT_H
