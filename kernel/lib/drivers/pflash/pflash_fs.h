#ifndef GZOS_PFLASH_FS_H
#define GZOS_PFLASH_FS_H
#ifdef __cplusplus

#include <platform/malta/mips.h>
#include <platform/malta/malta.h>
#include <vector>
#include <lib/kernel/vfs/VFSNode.h>

struct PFlashPartition {
    const char* name;
    uintptr_t offset;
    uint32_t size;
};

class PFlashVFSNode : public BasicVFSNode
{
public:
    PFlashVFSNode(const PFlashPartition& partition);
    virtual ~PFlashVFSNode(void) = default;

    virtual std::unique_ptr<FileDescriptor> open(void) override;
    virtual const std::vector<SharedNode>& childNodes(void) override;
    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) override;
    virtual size_t getSize(void) const override;

private:
    const uintptr_t _flash_base = MIPS_PHYS_TO_KSEG1(MALTA_FLASH_BASE);

    const PFlashPartition _partition;
};

class PFlashFileSystem : public BasicVFSNode
{
public:
    PFlashFileSystem(std::string&& path);
    virtual ~PFlashFileSystem(void) = default;

    virtual SharedNode createNode(VFSNode::Type type, std::string&& path) override;
    virtual const std::vector<SharedNode>& childNodes(void) override;
    virtual std::unique_ptr<FileDescriptor> open(void) override;

private:
    const PFlashPartition _partitions[2] = {
            {
                    .name = "yamon",
                    .offset = 0x0,
                    .size = 0x100000
            },
            {
                    .name = "userdata",
                    .offset = 0x100000,
                    .size = 0x300000
            }
    };

    std::vector<SharedNode> _nodes;
};

#endif //cplusplus

#endif //GZOS_PFLASH_FS_H
