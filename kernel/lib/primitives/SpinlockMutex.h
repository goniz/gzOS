//
// Created by gz on 6/12/16.
//

#ifndef GZOS_SPINLOCK_MUTEX_H
#define GZOS_SPINLOCK_MUTEX_H

#include <atomic>
#include "lib/primitives/AbstractMutex.h"

class SpinlockMutex : public AbstractMutex
{
public:
    SpinlockMutex(void) = default;
    virtual ~SpinlockMutex() = default;

private:
    virtual void onLockFailed(void) override;
    virtual void onUnlock(void) override;
};

#endif //GZOS_SPINLOCK_MUTEX_H
