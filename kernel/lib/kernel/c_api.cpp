
#include <platform/clock.h>
#include <lib/syscall/syscall.h>
#include <vfs/AccessControlledFileDescriptor.h>
#include "sched/scheduler.h"

extern "C" {

void scheduler_run_main(init_main_t init_main, void* argument)
{
    Scheduler::instance().createKernelThread("init", init_main, argument, 8192);
    clock_set_handler(Scheduler::onTickTimer, &Scheduler::instance());
}

int scheduler_signal_process(pid_t pid, int signal, uintptr_t value) {
    InterruptsMutex mutex;
    mutex.lock();
    return Scheduler::instance().signalPid(pid, signal, value);
}

int scheduler_set_timeout(int timeout_ms, timeout_callback_t callback, void* arg) {
    return Scheduler::instance().setTimeout(timeout_ms, (Scheduler::TimeoutCallbackFunc) callback, arg);
}

void scheduler_sleep(int timeout_ms) {
    Scheduler::instance().sleep(PID_CURRENT, timeout_ms);
}

void scheduler_suspend(void) {
    Scheduler::instance().suspend(PID_CURRENT);
}

void scheduler_resume(pid_t pid, uintptr_t value) {
    Scheduler::instance().resume(pid, value);
}

pid_t scheduler_current_pid(void)
{
    return Scheduler::instance().getCurrentPid();
}

}

int scheduler_syscall_handler(struct user_regs **regs, const struct kernel_syscall* syscall, va_list args) {
    return Scheduler::instance().syscall_entry_point(regs, syscall, args);
}

// extern "C"
