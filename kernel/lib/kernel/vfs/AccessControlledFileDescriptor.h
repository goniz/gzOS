#ifndef GZOS_ACCESSCONTROLLEDFILEDESCRIPTOR_H
#define GZOS_ACCESSCONTROLLEDFILEDESCRIPTOR_H

#ifdef __cplusplus

#include <bits/unique_ptr.h>
#include "FileDescriptor.h"

class AccessControlledFileDescriptor : public FileDescriptor {
public:
    AccessControlledFileDescriptor(std::unique_ptr<FileDescriptor> fd, int flags);
    virtual ~AccessControlledFileDescriptor(void) = default;

    virtual int read(void *buffer, size_t size) override;
    virtual int write(const void *buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual int stat(struct stat *stat) override;
    virtual void close(void) override;

private:
    std::unique_ptr<FileDescriptor> _fd;
    bool _writable;
    bool _readable;
};

#endif //cplusplus
#endif //GZOS_ACCESSCONTROLLEDFILEDESCRIPTOR_H
