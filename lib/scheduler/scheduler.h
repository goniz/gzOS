//
// Created by gz on 6/11/16.
//

#ifndef GZOS_SCHEDULER_H
#define GZOS_SCHEDULER_H

#ifdef __cplusplus
#include <vector>
#include <memory>
#include <lib/primitives/spinlock_mutex.h>
#include <lib/scheduler/process.h>
#include <lib/primitives/queue.h>
#include <platform/interrupts.h>
#endif

#define DefaultPreemptiveQuantum 100
#define DefaultResponsiveQuantum 1500

#ifdef __cplusplus

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
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*init_main_t)(int argc, const char** argv);
void scheduler_init(size_t initialProcSize, size_t initialQueueSize, init_main_t init_main, int argc, const char** argv);

#ifdef __cplusplus
};
#endif

#endif //GZOS_SCHEDULER_H
