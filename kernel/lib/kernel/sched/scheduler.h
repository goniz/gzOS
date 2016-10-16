#ifndef GZOS_SCHEDULER_H
#define GZOS_SCHEDULER_H

#ifdef __cplusplus
#include <vector>
#include <memory>
#include <lib/primitives/spinlock_mutex.h>
#include <lib/kernel/proccess/process.h>
#include <lib/primitives/basic_queue.h>
#include <platform/interrupts.h>
#endif

#include <lib/syscall/syscall.h>
#include <stdarg.h>
#include <sys/types.h>
#include "lib/kernel/sched/thread.h"

#define DefaultQuantum              (100)
#define SCHED_INITIAL_PROC_SIZE     (20)
#define SCHED_INITIAL_QUEUE_SIZE    (10)
#define SCHED_INITIAL_TIMERS_SIZE   (200)
#define PID_CURRENT                 (-512)
#define PID_PROCESS_START           (1)
#define PID_PROCESS_END             (254)
#define PID_THREAD_START            (1000)
#define PID_THREAD_END              (1999)

#ifdef __cplusplus

extern "C" {
int sys_ps(struct user_regs **regs, va_list args);
int scheduler_syscall_handler(struct user_regs **regs, const struct kernel_syscall *syscall, va_list args);
}

class Scheduler
{
public:
    using TimeoutCallbackFunc = bool (*)(Scheduler* self, void* arg);

    Scheduler(void);

    Process* createProcess(const char* name,
                        const void* buffer, size_t size,
                        std::vector<const char*>&& arguments,
                        size_t stackSize);

    Thread* createThread(Process& process,
                       const char* name,
                       Thread::EntryPointFunction entryPoint, void* argument,
                       size_t stackSize);

    Thread* createKernelThread(const char* name,
                               Thread::EntryPointFunction entryPoint, void* argument,
                               size_t stackSize);

    Process *CurrentProcess(void) const;
    Process* getProcessByPid(pid_t pid) const;
    Thread* CurrentThread(void) const;
    Thread* getThreadByTid(pid_t tid) const;
    pid_t getCurrentPid(void) const;
    pid_t getCurrentTid(void) const;

    bool signalPid(pid_t pid, int signal);
    bool setTimeout(int timeout_ms, TimeoutCallbackFunc cb, void* arg);
    void sleep(pid_t pid, int ms);

    const std::vector<std::unique_ptr<Process>>& processList(void) const {
        return _processList;
    }

    void setDebugMode(void);

    struct user_regs* yield(struct user_regs* regs);
    struct user_regs* schedule(struct user_regs* regs);
    static struct user_regs* onTickTimer(void* argument, struct user_regs* regs);

    void suspend(pid_t pid);
    void resume(pid_t pid);

    static Scheduler& instance(void);
    static bool isProcessPid(pid_t pid) {
        return (pid >= PID_PROCESS_START && pid <= PID_PROCESS_END);
    }

    static bool isThreadPid(pid_t pid) {
        return (pid >= PID_THREAD_START && pid <= PID_THREAD_END);
    }

private:
    Thread* andTheWinnerIs(void);
    bool signalThreadPid(pid_t pid, int signal);
    bool signalThread(Thread* thread, int signal);
    bool signalProcessPid(pid_t pid, int signal);
    void handleSignal(Thread* thread);
    void doTimers(void);
    int syscall_entry_point(struct user_regs **regs, const struct kernel_syscall *syscall, va_list args);
    int handleIRQDisabledSyscall(struct user_regs **regs, const kernel_syscall *syscall, va_list args);
    int handleIRQEnabledSyscall(struct user_regs **regs, const kernel_syscall *syscall, va_list args);
    friend int ::scheduler_syscall_handler(struct user_regs **regs, const struct kernel_syscall* syscall, va_list args);
    friend int ::sys_ps(struct user_regs **regs, va_list args);

    struct TimerControlBlock {
        uint64_t timeout_ms;
        uint64_t target_ms;
        TimeoutCallbackFunc callbackFunc;
        void* arg;
    };

    Thread*                                 _currentThread;
    basic_queue<Thread*>                    _readyQueue;
    Process*                                _kernelProc;
    Thread*                                 _idleThread;
    std::vector<std::unique_ptr<Process>>   _processList;
    std::vector<struct TimerControlBlock>   _timers;
    spinlock_mutex                          _mutex;
    std::atomic_bool                        _debugMode;
};

struct ps_ent {
    pid_t pid;
    int exit_code;
    uint32_t cpu_time;
    const char* state;
    char name[64];
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*init_main_t)(void* argument);
void scheduler_run_main(init_main_t init_main, void* argument);
int scheduler_syscall_handler(struct user_regs **regs, const struct kernel_syscall* syscall, va_list args);

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

pid_t gettid(void);

#ifdef __cplusplus
};
#endif

#endif //GZOS_SCHEDULER_H
