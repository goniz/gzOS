//
// Created by gz on 6/12/16.
//

#ifndef GZOS_INTERRUPTS_GUARD_H
#define GZOS_INTERRUPTS_GUARD_H

#include <cstdint>
#include <platform/interrupts.h>
#include <lib/primitives/Mutex.h>

class InterruptsMutex : public Mutex
{
public:
    InterruptsMutex(void)
        : Mutex(),
          _isLocked(false),
          _isrMask(0)
    {

    }

    InterruptsMutex(bool lockNow)
        : InterruptsMutex()
    {
        if (lockNow) {
            this->lock();
        }
    }

    ~InterruptsMutex(void) {
        if (_isLocked) {
            this->unlock();
        }
    }

    virtual void lock(void) override {
        if (!_isLocked) {
            _isrMask = interrupts_disable();
            _isLocked = true;
        }
    }

    virtual void unlock(void) override {
        if (_isLocked) {
            interrupts_enable(_isrMask);
            _isLocked = false;
        }
    }

private:
    bool     _isLocked;
    uint32_t _isrMask;
};

#endif //GZOS_INTERRUPTS_GUARD_H
