//
// Created by gz on 6/12/16.
//

#include <lib/primitives/spinlock_mutex.h>

spinlock_mutex::spinlock_mutex(void)
    : _value(State::Unlocked)
{

}

void spinlock_mutex::lock(void)
{
    State oldValue = State::Unlocked;
    while (!_value.compare_exchange_weak(oldValue, State::Locked));
}

void spinlock_mutex::unlock(void)
{
    _value.store(State::Unlocked);
}
