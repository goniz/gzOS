#ifndef GZOS_PIPE_H
#define GZOS_PIPE_H

#include <memory>
#include <vector>
#include <lib/kernel/vfs/FileDescriptor.h>
#include <lib/primitives/basic_queue.h>

#ifdef __cplusplus

class Pipe
{
public:
    static std::pair<UniqueFd, UniqueFd> create(void);

    ~Pipe(void) = default;

    int read(void* buffer, size_t size);
    int write(const void* buffer, size_t size);

private:
    Pipe(void);

    basic_queue<std::vector<uint8_t>> _bufferQueue;
};


#endif //cplusplus
#endif //GZOS_PIPE_H
