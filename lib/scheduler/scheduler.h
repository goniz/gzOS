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
#include <lib/primitives/basic_queue.h>
#include <platform/interrupts.h>
#endif

#define DefaultPreemptiveQuantum    100
#define DefaultResponsiveQuantum    1500
#define SCHED_INITIAL_PROC_SIZE     10
#define SCHED_INITIAL_QUEUE_SIZE    10

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

    struct user_regs* yield(struct user_regs* regs);

    bool signalProc(pid_t pid, int signal);
    Process* getProcessByPid(pid_t pid) const;

    pid_t getCurrentPid(void) const {
        return (_currentProc ? _currentProc->_pid : -1);
    }

    const std::vector<std::unique_ptr<Process>>& processList(void) const {
        return _processList;
    }

    void setDebugMode(void);

    static struct user_regs* onTickTimer(void* argument, struct user_regs* regs);

private:
    struct user_regs* schedule(struct user_regs* regs);
    Process* andTheWinnerIs(void);
    void handleSignal(Process *proc);
    Process* handleResponsiveProc(void);
    Process* handlePreemptiveProc(void);
    friend int sys_ps(struct user_regs **regs, va_list args);

    Process*                                _currentProc;
    basic_queue<Process*>                   _responsiveQueue;
    basic_queue<Process*>                   _preemptiveQueue;
    Process                                 _idleProc;
    std::vector<std::unique_ptr<Process>>   _processList;
    spinlock_mutex                          _mutex;
    bool                                    _debugMode;
};

struct ps_ent {
    pid_t pid;
    int exit_code;
    const char* state;
    const char* type;
    char name[64];
};

ProcessScheduler* scheduler(void);

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*init_main_t)(int argc, const char** argv);
void scheduler_run_main(init_main_t init_main, int argc, const char** argv);

#ifdef __cplusplus
};
#endif

#endif //GZOS_SCHEDULER_H
