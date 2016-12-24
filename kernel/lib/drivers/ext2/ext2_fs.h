#ifndef GZOS_EXT2_FS_H
#define GZOS_EXT2_FS_H

#include <lib/kernel/vfs/VirtualFileSystem.h>

#ifdef __cplusplus

class Ext2FileSystem : public FileSystem
{
    friend class Ext2ReaddirFileDescriptor;

public:
    Ext2FileSystem(const char* sourceDevice);
    virtual std::unique_ptr<FileDescriptor> open(const char *path, int flags) override;

    std::unique_ptr<FileDescriptor> readdir(const char* path) override;

private:
    void readSuperblock(void);
    void readBlockgroup(void);
    void readInodesTable(void);
    void readDirEntry(struct os_direntry_t& dirEntry, off_t offset, int whence);
    int findInodeByName(int baseInode, const char* filename);

    std::unique_ptr<FileDescriptor> _sourceFd;
    struct os_superblock_t* _superblock;
    struct os_blockgroup_descriptor_t* _blockgroup;
    struct os_inode_t* _inodes;
};

#endif //cplusplus
#endif //GZOS_EXT2_FS_H
