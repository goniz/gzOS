#ifndef GZOS_PFLASH_H
#define GZOS_PFLASH_H

#include <lib/kernel/vfs/FileDescriptor.h>

#ifdef __cplusplus

class PFlashFileDescriptor : public FileDescriptor {
public:
    PFlashFileDescriptor(uintptr_t start, uintptr_t end);

    virtual int read(void* buffer, size_t size) override;
    virtual int write(const void* buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual void close(void) override;

private:
    size_t size_left(void) {
        return _end - (_start + this->offset);
    }

    uintptr_t _start;
    uintptr_t _end;
};

#endif //cplusplus

#endif //GZOS_PFLASH_H
