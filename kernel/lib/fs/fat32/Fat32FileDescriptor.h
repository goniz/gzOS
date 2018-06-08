#ifndef GZOS_FAT32FILEDESCRIPTOR_H
#define GZOS_FAT32FILEDESCRIPTOR_H
#ifdef __cplusplus

#include <lib/kernel/vfs/FileDescriptor.h>
#include "ff.h"

class Fat32FileDescriptor : public FileDescriptor {
public:
    Fat32FileDescriptor(const char* name, int flags);
    virtual ~Fat32FileDescriptor(void) override;

    virtual const char* type(void) const override;
    virtual int read(void *buffer, size_t size) override;
    virtual int write(const void *buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual int stat(struct stat *stat) override;
    virtual void close(void) override;

private:
    uint8_t translateFlags(int flags) const;

    std::string _filename;
    FIL _fp;
};


#endif //cplusplus
#endif //GZOS_FAT32FILEDESCRIPTOR_H
