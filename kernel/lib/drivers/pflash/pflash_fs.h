#ifndef GZOS_PFLASH_FS_H
#define GZOS_PFLASH_FS_H

#include <lib/kernel/vfs/VirtualFileSystem.h>
#include <platform/malta/mips.h>
#include <platform/malta/malta.h>

#ifdef __cplusplus

class PFlashFileSystem : public FileSystem
{
    friend class PFlashReaddirFileDescriptor;

public:
    using FileDescriptorFactory = std::unique_ptr<FileDescriptor> (*)(void);

    PFlashFileSystem(void);
    virtual std::unique_ptr<FileDescriptor> open(const char *path, int flags) override;

    std::unique_ptr<FileDescriptor> readdir(const char* path) override;

private:
    struct PFlashPartition {
        const char* name;
        uintptr_t offset;
        uint32_t size;
    };

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

    const uintptr_t _flash_base = MIPS_PHYS_TO_KSEG1(MALTA_FLASH_BASE);
};

#endif //cplusplus

#endif //GZOS_PFLASH_FS_H
