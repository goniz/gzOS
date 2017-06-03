#ifndef GZOS_EVENT_H
#define GZOS_EVENT_H

#include "Suspendable.h"
#include <atomic>

class Event : private Suspendable
{
public:
    Event(void);
    virtual ~Event(void) = default;

    void wait(void);
    void raise(void);
    void reset(void);
    bool is_set(void) const;

private:
    std::atomic_bool _flag;
};

#endif //GZOS_EVENT_H
