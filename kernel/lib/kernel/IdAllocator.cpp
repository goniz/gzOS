#include <lib/primitives/InterruptsMutex.h>
#include <algorithm>
#include <lib/primitives/LockGuard.h>
#include "IdAllocator.h"

IdAllocator::IdAllocator(const size_t start, const size_t end)
    : _start(start), _end(end),
      _allocated(end - start)
{

}

unsigned int IdAllocator::allocate(void)
{
    LockGuard guard(_mutex);

    const size_t size = _allocated.size();
    for (size_t i = 0; i < size; i++) {
        if (_allocated[i]) {
            continue;
        }

        _allocated[i] = true;
        return (_start + i);
    }

    return -1;
}

void IdAllocator::deallocate(const unsigned int id)
{
    LockGuard guard(_mutex);

    if (!this->isAllocated(id)) {
        return;;
    }

    _allocated[id - _start] = false;
}

bool IdAllocator::isAllocated(const unsigned int id) {
    LockGuard guard(_mutex);

    return ((size_t)id >= _start && (size_t)id <= _end) && _allocated[id - _start];
}

