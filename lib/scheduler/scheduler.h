//
// Created by gz on 6/11/16.
//

#ifndef GZOS_SCHEDULER_H
#define GZOS_SCHEDULER_H

#include <vector>
#include <memory>
#include <lib/scheduler/process.h>
#include <lib/primitives/queue.h>
#include <platform/interrupts.h>

class ProcessScheduler
{
private:
    static constexpr uint32_t DefaultPreemptiveQuantum = 100;
    static constexpr uint32_t DefaultResponsiveQuantum = 1500;

public:
    ProcessScheduler(size_t initialProcSize, size_t initialQueueSize);

    static struct user_regs* onTickTimer(void* argument, struct user_regs* regs);

private:
    struct user_regs* schedule(struct user_regs* regs);
    Process* andTheWinnerIs(void);
    Process* handleResponsiveProc(void);
    Process* handlePreemptiveProc(void);

    Process*                                _currentProc;
    queue<Process*>                         _responsiveQueue;
    queue<Process*>                         _preemptiveQueue;
    Process                                 _idleProc;
    std::vector<std::unique_ptr<Process>>   _processList;
};

#endif //GZOS_SCHEDULER_H
