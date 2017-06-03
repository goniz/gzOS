#ifndef GZOS_PIPE_H
#define GZOS_PIPE_H

#include <memory>
#include <vector>
#include <lib/kernel/vfs/FileDescriptor.h>
#include <lib/primitives/basic_queue.h>
#include <lib/primitives/Event.h>

#ifdef __cplusplus

class Pipe
{
public:
    static std::pair<UniqueFd, UniqueFd> create(void);

    ~Pipe(void) = default;

    int read(void* buffer, size_t size);
    int write(const void* buffer, size_t size);
    int poll(bool* read_ready, bool* write_ready);

private:
    static constexpr int PipeMaxSize = 1*1024*1024;
    Pipe(void);
    bool isBufferEmptySafe(void);
    bool isBufferFullSafe();

    std::vector<uint8_t> _buffer;
    Event _readAvailable;
    Event _writeAvailable;
    SuspendableMutex _mutex;

    void processBufferSizeUnsafe();
};


#endif //cplusplus
#endif //GZOS_PIPE_H
