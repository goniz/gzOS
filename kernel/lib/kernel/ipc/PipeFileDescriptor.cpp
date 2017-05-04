#include "lib/kernel/ipc/PipeFileDescriptor.h"

PipeFileDescriptor::PipeFileDescriptor(std::shared_ptr<Pipe> pipe, Direction direction)
    : FileDescriptor(),
      _pipe(pipe),
      _direction(direction)
{

}

int PipeFileDescriptor::read(void* buffer, size_t size) {
    if (_direction != Direction::Read) {
        return -1;
    }

    return _pipe->read(buffer, size);
}

int PipeFileDescriptor::write(const void* buffer, size_t size) {
    if (_direction != Direction::Write) {
        return -1;
    }

    return _pipe->write(buffer, size);
}

int PipeFileDescriptor::seek(int where, int whence) {
    return -1;
}

int PipeFileDescriptor::stat(struct stat* stat) {
    return -1;
}
