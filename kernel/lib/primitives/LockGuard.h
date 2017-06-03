//
// Created by gz on 6/12/16.
//

#ifndef GZOS_LOCK_GUARD_H
#define GZOS_LOCK_GUARD_H

#include <lib/primitives/Mutex.h>

class LockGuard
{
public:
    LockGuard(Mutex& mutex)
        : _mutex(mutex)
    {
        _mutex.lock();
    }

    ~LockGuard(void) {
        _mutex.unlock();
    }

private:
    Mutex& _mutex;
};

#endif //GZOS_LOCK_GUARD_H
