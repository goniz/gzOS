#ifndef GZOS_SUSPENDABLE_H
#define GZOS_SUSPENDABLE_H


#include <sys/types.h>
#include <vector>
#include "interrupts_mutex.h"

class Suspendable {
public:
    Suspendable(void);
    Suspendable(const Suspendable&) = delete;
    Suspendable& operator=(const Suspendable& rhs) = delete;
    virtual ~Suspendable(void);

    void wait(void);
    void notifyOne(void);
    void notifyAll(void);

private:
    InterruptsMutex      m_mutex;
    std::vector<pid_t>   m_waitingPids;
};


#endif //GZOS_SUSPENDABLE_H
