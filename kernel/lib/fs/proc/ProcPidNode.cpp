#include <string>
#include <lib/fs/proc/ProcPidNode.h>

ProcPidNode::ProcPidNode(pid_t pid)
    : BasicVFSNode(Type::Directory, std::to_string(pid)),
      _pid(pid)
{

}

static const std::vector<SharedVFSNode> empty;
const std::vector<SharedVFSNode>& ProcPidNode::childNodes(void) {
    return empty;
}

SharedVFSNode ProcPidNode::createNode(VFSNode::Type type, std::string&& path) {
    return nullptr;
}

std::unique_ptr<FileDescriptor> ProcPidNode::open(void) {
    return nullptr;
}
