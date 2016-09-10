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
#define DefaultResponsiveQuantum    2500
#define SCHED_INITIAL_PROC_SIZE     10
#define SCHED_INITIAL_QUEUE_SIZE    10

#ifdef __cplusplus

extern "C"
int sys_ps(struct user_regs **regs, va_list args);

class ProcessScheduler
{
public:
    using TimeoutCallbackFunc = bool (*)(ProcessScheduler* self, void* arg);

    ProcessScheduler(size_t initialProcSize, size_t initialQueueSize);

    pid_t createPreemptiveProcess(const char* name,
                                  Process::EntryPointFunction main, std::vector<const char*>&& arguments,
                                  size_t stackSize, int initialQuantum = DefaultPreemptiveQuantum);
    pid_t createResponsiveProcess(const char* name,
                                  Process::EntryPointFunction main, std::vector<const char*>&& arguments,
                                  size_t stackSize, int initialQuantum = DefaultResponsiveQuantum);

    bool signalProc(pid_t pid, int signal);
    Process* getProcessByPid(pid_t pid) const;
    bool setTimeout(int timeout_ms, TimeoutCallbackFunc cb, void* arg);
    void sleep(pid_t pid, int ms);

    pid_t getCurrentPid(void) const {
        return (_currentProc ? _currentProc->_pid : -1);
    }

    const std::vector<std::unique_ptr<Process>>& processList(void) const {
        return _processList;
    }

    void setDebugMode(void);

    struct user_regs* yield(struct user_regs* regs);
    static struct user_regs* onTickTimer(void* argument, struct user_regs* regs);

    void suspend(pid_t pid);

    void resume(pid_t pid);

private:
    struct user_regs* schedule(struct user_regs* regs);
    Process* andTheWinnerIs(void);
    void handleSignal(Process *proc);
    Process* handleResponsiveProc(void);
    Process* handlePreemptiveProc(void);
    void doTimers(void);
    friend int ::sys_ps(struct user_regs **regs, va_list args);

    struct TimerControlBlock {
        uint64_t timeout_ms;
        uint64_t target_ms;
        TimeoutCallbackFunc callbackFunc;
        void* arg;
    };

    Process*                                _currentProc;
    basic_queue<Process*>                   _responsiveQueue;
    basic_queue<Process*>                   _preemptiveQueue;
    Process                                 _idleProc;
    std::vector<std::unique_ptr<Process>>   _processList;
    std::vector<struct TimerControlBlock>   _timers;
    spinlock_mutex                          _mutex;
    bool                                    _debugMode;
};

struct ps_ent {
    pid_t pid;
    int exit_code;
    uint32_t cpu_time;
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

typedef enum {
    TIMER_THATS_ENOUGH = 0,
    TIMER_KEEP_GOING = 1
} timeout_callback_ret;

typedef timeout_callback_ret (*timeout_callback_t)(void* arg);
int scheduler_set_timeout(int timeout_ms, timeout_callback_t callback, void* arg);

void scheduler_sleep(int timeout_ms);
void scheduler_suspend(void);
void scheduler_resume(pid_t pid);

pid_t scheduler_current_pid(void);
int scheduler_signal_process(pid_t pid, int signal);

#ifdef __cplusplus
};
#endif

#endif //GZOS_SCHEDULER_H
