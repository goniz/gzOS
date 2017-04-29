#ifndef GZOS_SCHEDULER_H
#define GZOS_SCHEDULER_H

#ifdef __cplusplus
#include <vector>
#include <memory>
#include <lib/primitives/spinlock_mutex.h>
#include <lib/kernel/proc/Process.h>
#include <lib/primitives/basic_queue.h>
#include <platform/interrupts.h>
#endif

#include <lib/syscall/syscall.h>
#include <stdarg.h>
#include <sys/types.h>
#include <lib/kernel/proc/ProcessProvider.h>
#include "Thread.h"
#include "SchedulingPolicy.h"

#define SCHED_INITIAL_TIMERS_SIZE   (200)
#define PID_CURRENT                 (-512)
#define PID_PROCESS_START           (1)
#define PID_PROCESS_END             (254)
#define PID_THREAD_START            (1000)
#define PID_THREAD_END              (1999)

#ifdef __cplusplus

extern "C" {
    int scheduler_syscall_handler(struct user_regs **regs, const struct kernel_syscall *syscall, va_list args);
}

class Scheduler
{
public:
    using TickCallbackFunc = bool (*)(void* argument, struct user_regs** regs);

    Scheduler(ProcessProvider& processProvider,
              std::unique_ptr<SchedulingPolicy> policy);

    void initialize(void);

    bool addThread(Thread* thread);
    bool removeThread(Thread* thread);
    bool addTickHandler(TickCallbackFunc func, void* argument);
    bool removeTickHandler(TickCallbackFunc func);

    Process* CurrentProcess(void) const;
    Thread* CurrentThread(void) const;

    pid_t getCurrentPid(void) const;
    pid_t getCurrentTid(void) const;

    void setDebugMode(void);

    struct user_regs* schedule(struct user_regs* regs);
    static struct user_regs* onTickTimer(void* argument, struct user_regs* regs);

    bool suspend(Thread* thread);
    bool suspend(Process* process);
    bool resume(Thread* thread, uintptr_t value);
    bool resume(Process* process, uintptr_t value);
    bool kill(Thread* thread, bool yield = true);
    bool kill(Process* process, bool yield = true);

    int waitForPid(pid_t pid);

    static Scheduler& instance(void);
    static bool isProcessPid(pid_t pid) {
        return (pid >= PID_PROCESS_START && pid <= PID_PROCESS_END);
    }

    static bool isThreadPid(pid_t pid) {
        return (pid >= PID_THREAD_START && pid <= PID_THREAD_END);
    }

private:
    bool destroyProcess(Process* proc);

    int syscall_entry_point(struct user_regs **regs, const struct kernel_syscall *syscall, va_list args);
    int handleIRQDisabledSyscall(struct user_regs **regs, const kernel_syscall *syscall, va_list args);
    int handleIRQEnabledSyscall(struct user_regs **regs, const kernel_syscall *syscall, va_list args);

    friend int ::scheduler_syscall_handler(struct user_regs **regs, const struct kernel_syscall* syscall, va_list args);

    struct TickControlBlock {
        TickCallbackFunc function;
        void* argument;
    };

    Thread*                                 _currentThread;
    ProcessProvider&                        _processProvider;
    std::unique_ptr<SchedulingPolicy>      _policy;
    std::vector<TickControlBlock>           _tickHandlers;
    Thread*                                 _idleThread;
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

typedef timeout_callback_ret (*timeout_callback_t)(void* scheduler, void* arg);
uint32_t scheduler_set_timeout(int timeout_ms, timeout_callback_t callback, void* arg);
void scheduler_unset_timeout(uint32_t timeoutId);

pid_t scheduler_current_pid(void);
pid_t scheduler_current_tid(void);

#ifdef __cplusplus
};
#endif

#endif //GZOS_SCHEDULER_H
