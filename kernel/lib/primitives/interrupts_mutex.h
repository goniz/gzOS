//
// Created by gz on 6/12/16.
//

#ifndef GZOS_INTERRUPTS_GUARD_H
#define GZOS_INTERRUPTS_GUARD_H

#include <cstdint>
#include <platform/interrupts.h>

class InterruptsMutex
{
public:
    InterruptsMutex(void) : _isLocked(false), _isrMask(0) {}
    InterruptsMutex(bool lockNow) : InterruptsMutex() {
        if (lockNow) {
            this->lock();
        }
    }

    ~InterruptsMutex(void) {
        if (_isLocked) {
            this->unlock();
        }
    }

    void lock(void) {
        if (!_isLocked) {
            _isrMask = interrupts_disable();
            _isLocked = true;
        }
    }

    void unlock(void) {
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