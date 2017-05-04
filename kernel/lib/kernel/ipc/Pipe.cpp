#include <sys/param.h>
#include "lib/kernel/ipc/Pipe.h"
#include "PipeFileDescriptor.h"

Pipe::Pipe(void)
    : _bufferQueue()
{
    _bufferQueue.reserve(10);
}

int Pipe::read(void* buffer, size_t size) {

    std::vector<uint8_t> head;
    if (!_bufferQueue.peek(head, true)) {
        return -1;
    }

    InterruptsMutex mutex(true);
    size_t size_to_copy = MIN(head.size(), size);
    std::copy_n(head.begin(), size_to_copy, (uint8_t*)buffer);
    head.erase(head.begin(), head.begin() + size_to_copy);

    if (head.empty()) {
        _bufferQueue.pop(head, false);
    }

    return size_to_copy;
}

int Pipe::write(const void* buffer, size_t size) {
    InterruptsMutex mutex(true);

    if (_bufferQueue.empty()) {
        if (_bufferQueue.push(std::vector<uint8_t>((uint8_t*)buffer, (uint8_t*)buffer + size))) {
            return size;
        }

        return -1;
    }

    std::vector<uint8_t> back;
    if (!_bufferQueue.peek_back(back, false)) {
        if (_bufferQueue.push(std::vector<uint8_t>((uint8_t*)buffer, (uint8_t*)buffer + size))) {
            return size;
        }

        return -1;
    }

    std::copy_n((uint8_t*)buffer, size, back.end());
    return size;
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