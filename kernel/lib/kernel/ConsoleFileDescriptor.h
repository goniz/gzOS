#ifndef GZOS_CONSOLEDEVICE_H
#define GZOS_CONSOLEDEVICE_H

#ifdef __cplusplus
#include "FileDescriptor.h"

class ConsoleFileDescriptor : public FileDescriptor {
public:
    virtual int read(void* buffer, size_t size) override;
    virtual int write(const void* buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual void close(void) override;
};

#endif //cplusplus

#endif //GZOS_CONSOLEDEVICE_H
