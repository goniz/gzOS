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

void PipeFileDescriptor::close(void) {
    kprintf("Closing pipe direction %s\n", _direction == Pipe::Direction::Read ? "Read" : "Write");
    _pipe->close(_direction);
}

int PipeFileDescriptor::poll(bool* read_ready, bool* write_ready) {
    if (Direction::Write == _direction) {
        read_ready = nullptr;
    } else {
        write_ready = nullptr;
    }

    return _pipe->poll(read_ready, write_ready);
}

int PipeFileDescriptor::seek(int where, int whence) {
    return -1;
}

int PipeFileDescriptor::stat(struct stat* stat) {
    return -1;
}

const char* PipeFileDescriptor::type(void) const {
    return "PipeFileDescriptor";
}
