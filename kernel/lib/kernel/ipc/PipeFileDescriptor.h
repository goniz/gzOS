#ifndef GZOS_PIPEFILEDESCRIPTOR_H
#define GZOS_PIPEFILEDESCRIPTOR_H

#ifdef __cplusplus

#include <lib/kernel/vfs/FileDescriptor.h>
#include "lib/kernel/ipc/Pipe.h"

class PipeFileDescriptor : public FileDescriptor
{
public:
    using Direction = Pipe::Direction;

    PipeFileDescriptor(std::shared_ptr<Pipe> pipe, Direction direction);
    virtual ~PipeFileDescriptor(void) {
        this->close();
    }

    virtual const char* type(void) const override;
    virtual int read(void* buffer, size_t size) override;
    virtual int write(const void* buffer, size_t size) override;
    virtual int stat(struct stat* stat) override;
    virtual int seek(int where, int whence) override;
    virtual void close(void) override;

    int poll(bool* read_ready, bool* write_ready) override;

private:
    std::shared_ptr<Pipe> _pipe;
    Direction _direction;
};

#endif //cplusplus
#endif //GZOS_PIPEFILEDESCRIPTOR_H
