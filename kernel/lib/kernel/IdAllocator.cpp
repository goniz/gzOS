#include <lib/primitives/interrupts_mutex.h>
#include <algorithm>
#include "IdAllocator.h"

IdAllocator::IdAllocator(const size_t start, const size_t end)
    : _start(start), _end(end),
      _allocated(end - start)
{

}

int IdAllocator::allocate(void)
{
    InterruptsMutex mutex(true);

    const size_t size = _allocated.size();
    for (size_t i = 0; i < size; i++) {
        if (_allocated[i]) {
            continue;
        }

        _allocated[i] = true;
        return (int) (_start + i);
    }

    return -1;
}

void IdAllocator::deallocate(const int id)
{
    InterruptsMutex mutex(true);
    if (!isAllocated(id)) {
        return;;
    }

    _allocated[id - _start] = false;
}

bool IdAllocator::isAllocated(const int id) {
    InterruptsMutex mutex(true);
    return ((size_t)id >= _start && (size_t)id <= _end) && _allocated[id - _start];
}

