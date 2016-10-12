//
// Created by gz on 6/12/16.
//

#ifndef GZOS_LOCK_GUARD_H
#define GZOS_LOCK_GUARD_H

template<typename TMutex>
class lock_guard
{
public:
    lock_guard(TMutex& mutex) : _mutex(mutex) {
        _mutex.lock();
    }

    ~lock_guard(void) {
        _mutex.unlock();
    }

private:
    TMutex& _mutex;
};

#endif //GZOS_LOCK_GUARD_H
