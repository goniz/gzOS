#ifndef GZOS_FAT32_FS_H
#define GZOS_FAT32_FS_H

#include <lib/kernel/vfs/BasicVFSNode.h>
#include "ff.h"

#ifdef __cplusplus

class Fat32VFSNode : public BasicVFSNode
{
public:
    Fat32VFSNode(VFSNode::Type type, std::string&& fullPath, std::string&& segment);
    virtual ~Fat32VFSNode(void) = default;

    virtual std::unique_ptr<FileDescriptor> open(void) override;
    virtual const std::vector<SharedVFSNode>& childNodes(void) override;
    virtual SharedVFSNode createNode(VFSNode::Type type, std::string&& path) override;
    virtual size_t getSize(void) const override;

protected:
    Path getCurrentFatPath(void) const;
    virtual bool isRootNode(void) const;

private:
    std::vector<SharedVFSNode> _nodes;
    std::string _parentFullPath;
};

class Fat32FileSystem : public Fat32VFSNode
{
public:
    Fat32FileSystem(const char* sourceDevice, std::string&& path);
    virtual ~Fat32FileSystem(void) = default;

protected:
    virtual bool isRootNode(void) const override;

private:
    std::unique_ptr<FileDescriptor> _sourceFd;
    FATFS _fs;
};

#endif //cplusplus
#endif //GZOS_FAT32_FS_H
