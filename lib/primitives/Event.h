#ifndef GZOS_EVENT_H
#define GZOS_EVENT_H

#include "Suspendable.h"

class Event : private Suspendable
{
public:
    Event(void) = default;
    virtual ~Event(void) = default;

    void wait(void);
    void raise(void);
};

#endif //GZOS_EVENT_H
