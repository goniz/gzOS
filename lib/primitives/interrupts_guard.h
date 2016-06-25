//
// Created by gz on 6/12/16.
//

#ifndef GZOS_INTERRUPTS_GUARD_H
#define GZOS_INTERRUPTS_GUARD_H

#include <cstdint>
#include <platform/interrupts.h>

class InterruptGuard
{
public:
    InterruptGuard(void) {
        _isrMask = interrupts_disable();
    }

    ~InterruptGuard(void) {
        interrupts_enable(_isrMask);
    }

private:
    uint32_t _isrMask;
};

#endif //GZOS_INTERRUPTS_GUARD_H
