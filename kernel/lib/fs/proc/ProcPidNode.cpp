#include <string>
#include <lib/fs/proc/ProcPidNode.h>
#include <lib/kernel/proc/ProcessProvider.h>
#include <lib/kernel/vfs/TmpfsNode.h>
#include <algorithm>

ProcPidNode::ProcPidNode(pid_t pid)
    : BasicVFSNode(Type::Directory, std::to_string(pid)),
      _pid(pid),
      _nodes()
{
    Process* proc = ProcessProvider::instance().getProcessByPid(_pid);
    if (!proc) {
        return;
    }

    _nodes.push_back(std::make_shared<TmpfsFileNode>("name", proc->name()));
    auto cwd = proc->currentWorkingPath().string();
    _nodes.push_back(std::make_shared<TmpfsFileNode>("cwd", cwd.c_str()));
    auto cpu = std::to_string(proc->cpu_time());
    _nodes.push_back(std::make_shared<TmpfsFileNode>("cpu", cpu.c_str()));

    std::vector<uint8_t> cmdline;
    for (auto const& arg : proc->arguments()) {
        cmdline.insert(cmdline.end(), arg.c_str(), arg.c_str() + arg.size());
        cmdline.push_back(' ');
    }

    _nodes.push_back(std::make_shared<TmpfsFileNode>("cmdline", std::move(cmdline)));
}

SharedVFSNode ProcPidNode::createNode(VFSNode::Type type, std::string&& path) {
    return nullptr;
}

std::unique_ptr<FileDescriptor> ProcPidNode::open(void) {
    return nullptr;
}

const std::vector<SharedVFSNode>& ProcPidNode::childNodes(void) {
    return _nodes;
}