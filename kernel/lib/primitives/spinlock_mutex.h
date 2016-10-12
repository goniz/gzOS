//
// Created by gz on 6/12/16.
//

#ifndef GZOS_SPINLOCK_MUTEX_H
#define GZOS_SPINLOCK_MUTEX_H

#include <atomic>

class spinlock_mutex
{
private:
    enum State{Locked, Unlocked};

public:
    spinlock_mutex(void) : _value(State::Unlocked) {}

    void lock(void);
    void unlock(void);

private:
    std::atomic<State> _value;
};

#endif //GZOS_SPINLOCK_MUTEX_H
