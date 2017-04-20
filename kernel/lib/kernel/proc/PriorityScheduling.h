#ifndef GZOS_PRIORITY_SCHEDULING_H
#define GZOS_PRIORITY_SCHEDULING_H

#include <lib/primitives/basic_queue.h>
#include "SchedulingPolicy.h"
#include "Thread.h"

#ifdef __cplusplus



class PriorityQueuesSchedulingPolicy : public SchedulingPolicy
{

private:
    basic_queue<Thread*>                    _readyQueue;
};



#endif //cplusplus

#endif //GZOS_PRIORITY_SCHEDULING_H
