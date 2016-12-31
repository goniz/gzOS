#ifndef GZOS_PFLASH_H
#define GZOS_PFLASH_H

#include <lib/kernel/vfs/FileDescriptor.h>

#ifdef __cplusplus

class PFlashFileDescriptor : public MemoryBackedFileDescriptor {
public:
    PFlashFileDescriptor(uintptr_t start, uintptr_t end);

    virtual int write(const void* buffer, size_t size) override;
};

#endif //cplusplus

#endif //GZOS_PFLASH_H
