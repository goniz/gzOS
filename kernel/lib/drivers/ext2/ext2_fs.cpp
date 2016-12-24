#include <platform/drivers.h>
#include <platform/kprintf.h>
#include <platform/cpu.h>
#include <cassert>
#include <lib/kernel/vfs/ReaddirFileDescriptor.h>
#include <lib/primitives/interrupts_mutex.h>
#include <lib/primitives/hexdump.h>
#include "ext2_fs.h"
#include "inc/ext2access.h"

static void ls(VirtualFileSystem &vfs, const char* path)
{
    struct DirEntry dirent;
    int ret;
    auto readdirfd = vfs.readdir(path);
    if (nullptr == readdirfd) {
        printf("failed to create readdir handle\n");
        return;
    }

    printf("ls %s\n", path);
    while (0 < (ret = readdirfd->read(&dirent, sizeof(dirent)))) {
        printf("* %s%s\n", dirent.name, dirent.type == DirEntryType::DIR ? "/" : "");
    }

    printf("ret: %d\n", ret);
    readdirfd->close();
}

static int dev_ext2_init(void) {
    VirtualFileSystem &vfs = VirtualFileSystem::instance();

    vfs.registerFilesystem("ext2", [](const char *source) {
        return std::unique_ptr<FileSystem>(new Ext2FileSystem(source));
    });

    vfs.mountFilesystem("ext2", "/pflash/userdata", "/mnt/userdata");
    ls(vfs, "/mnt/userdata");


    return 0;
}

DECLARE_DRIVER(dev_ext2, dev_ext2_init, STAGE_SECOND + 1);

Ext2FileSystem::Ext2FileSystem(const char *sourceDevice)
        : _sourceFd(),
          _superblock(),
          _blockgroup(),
          _inodes() {
    VirtualFileSystem &vfs = VirtualFileSystem::instance();

    _sourceFd = vfs.open(sourceDevice, O_RDONLY);
    if (!_sourceFd) {
        kprintf("Could NOT open file \"%s\"\n", sourceDevice);
        throw std::exception();
    }

    this->readSuperblock();
    kprintf("block size \t\t= %d bytes\n", 1 << (10 + _superblock->s_log_block_size));
    kprintf("inode count \t\t= 0x%x\n", _superblock->s_inodes_count);
    kprintf("inode size \t\t= 0x%x\n", _superblock->s_inode_size);

    this->readBlockgroup();
    kprintf("inode table address \t= 0x%x\n", _blockgroup->bg_inode_table);
    kprintf("inode table size \t= %dKB\n", (_superblock->s_inodes_count * _superblock->s_inode_size) >> 10);

    this->readInodesTable();
}

void Ext2FileSystem::readSuperblock(void) {
    _superblock = (os_superblock_t *) malloc(sizeof(*_superblock));
    assert(_superblock != NULL);

    assert(_sourceFd->seek((off_t) 1024, SEEK_SET) == (off_t) 1024);
    assert(_sourceFd->read(_superblock, sizeof(struct os_superblock_t)) == sizeof(struct os_superblock_t));

    endian_set(le32_to_cpu, _superblock->s_inodes_count);
    endian_set(le32_to_cpu, _superblock->s_blocks_count);
    endian_set(le32_to_cpu, _superblock->s_r_blocks_count);
    endian_set(le32_to_cpu, _superblock->s_free_blocks_count);
    endian_set(le32_to_cpu, _superblock->s_free_inodes_count);
    endian_set(le32_to_cpu, _superblock->s_first_data_block);
    endian_set(le32_to_cpu, _superblock->s_log_block_size);
    endian_set(le32_to_cpu, _superblock->s_log_frag_size);
    endian_set(le32_to_cpu, _superblock->s_blocks_per_group);
    endian_set(le32_to_cpu, _superblock->s_frags_per_group);
    endian_set(le32_to_cpu, _superblock->s_inodes_per_group);
    endian_set(le32_to_cpu, _superblock->s_mtime);
    endian_set(le32_to_cpu, _superblock->s_wtime);
    endian_set(le16_to_cpu, _superblock->s_mnt_count);
    endian_set(le16_to_cpu, _superblock->s_max_mnt_count);
    endian_set(le16_to_cpu, _superblock->s_magic);
    endian_set(le16_to_cpu, _superblock->s_state);
    endian_set(le16_to_cpu, _superblock->s_errors);
    endian_set(le16_to_cpu, _superblock->s_minor_rev_level);
    endian_set(le32_to_cpu, _superblock->s_lastcheck);
    endian_set(le32_to_cpu, _superblock->s_checkinterval);
    endian_set(le32_to_cpu, _superblock->s_creator_os);
    endian_set(le32_to_cpu, _superblock->s_rev_level);
    endian_set(le16_to_cpu, _superblock->s_def_resuid);
    endian_set(le16_to_cpu, _superblock->s_def_resgid);
    endian_set(le32_to_cpu, _superblock->s_first_ino);
    endian_set(le16_to_cpu, _superblock->s_inode_size);
    endian_set(le16_to_cpu, _superblock->s_block_group_nr);
    endian_set(le32_to_cpu, _superblock->s_feature_compat);
    endian_set(le32_to_cpu, _superblock->s_feature_incompat);
    endian_set(le32_to_cpu, _superblock->s_feature_ro_compat);
    endian_set(le32_to_cpu, _superblock->s_algo_bitmap);
    endian_set(le32_to_cpu, _superblock->s_journal_inum);
    endian_set(le32_to_cpu, _superblock->s_journal_dev);
    endian_set(le32_to_cpu, _superblock->s_last_orphan);
    endian_set(le32_to_cpu, _superblock->s_default_mount_options);
    endian_set(le32_to_cpu, _superblock->s_first_meta_bg);
}

void Ext2FileSystem::readBlockgroup(void) {
    _blockgroup = (os_blockgroup_descriptor_t *) malloc(sizeof(*_blockgroup));
    assert(_blockgroup != NULL);

    assert(_sourceFd->seek((off_t) 2048, SEEK_SET) == (off_t) 2048);
    assert(_sourceFd->read(_blockgroup, sizeof(struct os_blockgroup_descriptor_t)) ==
           sizeof(struct os_blockgroup_descriptor_t));

    endian_set(le32_to_cpu, _blockgroup->bg_block_bitmap);
    endian_set(le32_to_cpu, _blockgroup->bg_inode_bitmap);
    endian_set(le32_to_cpu, _blockgroup->bg_inode_table);
    endian_set(le16_to_cpu, _blockgroup->bg_free_blocks_count);
    endian_set(le16_to_cpu, _blockgroup->bg_free_inodes_count);
    endian_set(le16_to_cpu, _blockgroup->bg_used_dirs_count);
}

void Ext2FileSystem::readInodesTable(void) {
    _inodes = (struct os_inode_t *) malloc(_superblock->s_inodes_count * _superblock->s_inode_size);
    assert(_inodes != NULL);

    // seek to start of inode_table
    off_t offset = (off_t) (_blockgroup->bg_inode_table * 1024);
    assert(_sourceFd->seek(offset, SEEK_SET) == offset);

    // read-in every inode into cache
    for (unsigned int i = 0; i < _superblock->s_inodes_count; i++) {
        _sourceFd->seek((off_t) _superblock->s_inode_size, SEEK_CUR);

        struct os_inode_t* inode = &_inodes[i];
        assert(_sourceFd->read(inode, sizeof(struct os_inode_t)) == sizeof(struct os_inode_t));

        endian_set(le16_to_cpu, inode->i_mode);
        endian_set(le16_to_cpu, inode->i_uid);
        endian_set(le32_to_cpu, inode->i_size);
        endian_set(le32_to_cpu, inode->i_atime);
        endian_set(le32_to_cpu, inode->i_ctime);
        endian_set(le32_to_cpu, inode->i_mtime);
        endian_set(le32_to_cpu, inode->i_dtime);
        endian_set(le16_to_cpu, inode->i_gid);
        endian_set(le16_to_cpu, inode->i_links_count);
        endian_set(le16_to_cpu, inode->i_blocks);
        endian_set(le32_to_cpu, inode->i_flags);

        for (int j = 0; j < 15; j++) {
            endian_set(le32_to_cpu, inode->i_block[j]);
        }

        endian_set(le32_to_cpu, inode->i_generation);
        endian_set(le32_to_cpu, inode->i_file_acl);
        endian_set(le32_to_cpu, inode->i_dir_acl);
        endian_set(le32_to_cpu, inode->i_faddr);
    }
}

void Ext2FileSystem::readDirEntry(struct os_direntry_t& dirEntry, off_t offset, int whence)
{
    memset(&dirEntry, 0, sizeof(dirEntry));

    int ret = _sourceFd->seek(offset, whence);
    if (SEEK_SET == whence) {
        assert(ret == offset);
    }

    assert(_sourceFd->read(&dirEntry, sizeof(struct os_direntry_t)) == sizeof(struct os_direntry_t));

    endian_set(le32_to_cpu, dirEntry.inode);
    endian_set(le16_to_cpu, dirEntry.rec_len);
    endian_set(le16_to_cpu, dirEntry.name_len);
}

/* findInodeByName
 *
 * Params:
 * int fd		fd to img file
 * char* filename	name of file to find
 * int filetype		filetype os_direntry_t->file_type
 *
 * Returns:
 * int			valid inode-num if found, else -1.
 */
int Ext2FileSystem::findInodeByName(int baseInode, const char* filename) {
    auto* inode = &_inodes[baseInode];

//    kprintf("baseInode: %d\n", baseInode);
//    kprintf("filename: %s\n", filename);
//    kprintf("inode: %p\n", inode);
//    kprintf("inode block0: %08x\n", inode[0].i_block[0]);

    struct os_direntry_t dirEntry;

    this->readDirEntry(dirEntry, (off_t) (inode->i_block[0] * 1024), SEEK_SET);

    while (dirEntry.inode) {
        char name[dirEntry.name_len + 1];
        memcpy(name, dirEntry.file_name, dirEntry.name_len);
        name[dirEntry.name_len] = '\0';

//        kprintf("dirent inode: %d\n", dirEntry.inode);
//        kprintf("dirent name: %s\n", name);
//        hexDump((char *) "dirent", &dirEntry, sizeof(dirEntry));

        if (0 == strcmp(name, filename)) {
            return dirEntry.inode;
        }

        this->readDirEntry(dirEntry, dirEntry.rec_len - sizeof(struct os_direntry_t), SEEK_CUR);
    }

    return(-1);
}

class Ext2ReaddirFileDescriptor : public ReaddirFileDescriptor {
public:
    Ext2ReaddirFileDescriptor(Ext2FileSystem& fs, int baseInode)
            : _baseInode(baseInode),
              _offset(0),
              _fs(fs)
    {
        // start at the dirent table of the base inode
        const auto* inode = &_fs._inodes[_baseInode];
        _offset = (off_t) (inode->i_block[0] * 1024);

//        kprintf("baseInode: %d\n", _baseInode);
//        kprintf("inode: %p\n", inode);
//        kprintf("inode block0: %08x\n", inode[0].i_block[0]);
//        kprintf("offset: %d\n", _offset);
    }

private:
    bool getNextEntry(struct DirEntry &dirEntry) override {
        InterruptsMutex mutex(true);
        struct os_direntry_t tmpDirEntry;

        readDirEntry(_offset, tmpDirEntry);

//        kprintf("dir inode: %d\n", tmpDirEntry.inode);
//        kprintf("dir name: %s\n", tmpDirEntry.file_name);

        if (0 == tmpDirEntry.inode) {
            return false;
        }

        kprintf("inode: %d\n", tmpDirEntry.inode);
        hexDump((char*)"inode:", &_fs._inodes[tmpDirEntry.inode - 1], sizeof(os_inode_t));

        auto mode = _fs._inodes[tmpDirEntry.inode - 1].i_mode;
        if (mode & EXT2_S_IFREG) {
            dirEntry.type = DirEntryType::REG;
        } else if (mode & EXT2_FT_DIR) {
            dirEntry.type = DirEntryType::DIR;
        }

        memcpy(dirEntry.name, tmpDirEntry.file_name, tmpDirEntry.name_len);
        dirEntry.name[tmpDirEntry.name_len] = '\0';

        kprintf("name: %s mode: %04x\n", dirEntry.name, mode);

        _offset += (tmpDirEntry.rec_len);
        return true;
    }

    void readDirEntry(off_t offset, struct os_direntry_t& direntry) {
        off_t savedOffset = _fs._sourceFd->get_offset();
        _fs.readDirEntry(direntry, offset, SEEK_SET);
        _fs._sourceFd->seek(savedOffset, SEEK_SET);
    }

    int _baseInode;
    off_t _offset;
    Ext2FileSystem& _fs;
};

std::unique_ptr<FileDescriptor> Ext2FileSystem::readdir(const char* path) {
    if (0 == strcmp("", path)) {
        path = ".";
    }

    int inode = this->findInodeByName(0, path);
    return std::make_unique<Ext2ReaddirFileDescriptor>(*this, inode - 2);
}

std::unique_ptr<FileDescriptor> Ext2FileSystem::open(const char *path, int flags) {
    // TODO: impelement
    return std::unique_ptr<FileDescriptor>();
}