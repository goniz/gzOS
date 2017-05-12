#ifndef GZOS_PROCPIDNODE_H
#define GZOS_PROCPIDNODE_H
#ifdef __cplusplus

#include <lib/kernel/vfs/BasicVFSNode.h>

class ProcPidNode : public BasicVFSNode
{
public:
    ProcPidNode(pid_t pid);
    virtual ~ProcPidNode(void) = default;

    std::unique_ptr<FileDescriptor> open(void) override;
    SharedVFSNode createNode(VFSNode::Type type, std::string&& path) override;
    const std::vector<SharedVFSNode>& childNodes(void) override;

private:
    pid_t _pid;
    std::vector<SharedVFSNode> _nodes;
};

#endif //cplusplus
#endif //GZOS_PROCPIDNODE_H
