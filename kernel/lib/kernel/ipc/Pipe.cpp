#include <sys/param.h>
#include <cassert>
#include "lib/kernel/ipc/Pipe.h"
#include "PipeFileDescriptor.h"

Pipe::Pipe(void)
    : _buffer(),
      _readAvailable(),
      _writeAvailable(),
      _mutex()
{
    _buffer.reserve(1024);
}

int Pipe::read(void* buffer, size_t size) {
    // if the read end is closed and read() was called
    // then return an error
    if (_readClosed) {
        return -1;
    }

    // if we dont have any data to read but the write side is still open
    // then we need to wait for data to be available
    while (this->isBufferEmptySafe() && !_writeClosed) {
        _readAvailable.wait();
    }

    // if the write side is closed
    // then return EOF
    if (_writeClosed) {
        return 0;
    }

    LockGuard guard(_mutex);

    size_t size_to_copy = MIN(_buffer.size(), size);
    assert(0 != size_to_copy);

    std::copy_n(_buffer.begin(), size_to_copy, (uint8_t*)buffer);
    _buffer.erase(_buffer.begin(), _buffer.begin() + size_to_copy);

    this->processBufferSizeUnsafe();

    return size_to_copy;
}

int Pipe::write(const void* buffer, size_t size) {
    if (_writeClosed || _readClosed) {
        return -1;
    }

    while (this->isBufferFullSafe()) {
        _writeAvailable.wait();
    }

    LockGuard guard(_mutex);

    _buffer.insert(_buffer.end(), (char*)buffer, (char*)buffer + size);

    this->processBufferSizeUnsafe();

    return size;
}

int Pipe::poll(bool* read_ready, bool* write_ready) {
    if (read_ready) {
        *read_ready = !this->isBufferEmptySafe() || _writeClosed || _readClosed;
    }

    if (write_ready) {
        *write_ready = !this->isBufferFullSafe() || _writeClosed || _readClosed;
    }

    return 0;
}

bool Pipe::isBufferEmptySafe(void) {
    LockGuard guard(_mutex);
    return _buffer.empty();
}

bool Pipe::isBufferFullSafe() {
    LockGuard guard(_mutex);
    return _buffer.size() >= Pipe::PipeMaxSize;
}

void Pipe::processBufferSizeUnsafe() {
    // if the buffer is empty
    if (_buffer.empty()) {
        // we want to signal that read is unavailable
        _readAvailable.reset();
    } else {
        // else, let them read!
        _readAvailable.raise();
    }

    // if the buffer is less than max size
    if (_buffer.size() < Pipe::PipeMaxSize) {
        // mark write as available
        _writeAvailable.raise();
    } else {
        // else mark as full
        _writeAvailable.reset();
    }
}

void Pipe::close(Pipe::Direction direction) {
    if (direction == Direction::Read) {
        _readClosed = true;
    }

    if (direction == Direction::Write) {
        _writeClosed = true;
    }
}

std::pair<UniqueFd, UniqueFd> Pipe::create(void) {
    std::shared_ptr<Pipe> pipe(new Pipe());
    if (!pipe) {
        return { nullptr, nullptr };
    }

    return {
            std::make_unique<PipeFileDescriptor>(pipe, PipeFileDescriptor::Direction::Read),
            std::make_unique<PipeFileDescriptor>(pipe, PipeFileDescriptor::Direction::Write)
    };
}
