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
#include <lib/primitives/spinlock_mutex.h>

#define DefaultPreemptiveQuantum 100
#define DefaultResponsiveQuantum 1500

class ProcessScheduler
{
private:

public:
    ProcessScheduler(size_t initialProcSize, size_t initialQueueSize);

    pid_t createPreemptiveProcess(const char* name,
                                  Process::EntryPointFunction main, std::vector<const char*>&& arguments,
                                  size_t stackSize, int initialQuantum = DefaultPreemptiveQuantum);
    pid_t createResponsiveProcess(const char* name,
                                  Process::EntryPointFunction main, std::vector<const char*>&& arguments,
                                  size_t stackSize, int initialQuantum = DefaultResponsiveQuantum);

    void setDebugMode(void);

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
    spinlock_mutex                          _mutex;
    bool                                    _debugMode;
};

#endif //GZOS_SCHEDULER_H
