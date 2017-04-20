
#include <platform/clock.h>
#include <lib/syscall/syscall.h>
#include <vfs/AccessControlledFileDescriptor.h>
#include <proc/proc.h>
#include "proc/Scheduler.h"

extern "C" {

void scheduler_run_main(init_main_t init_main, void* argument)
{
    ProcessProvider::instance().createKernelThread("kernel_main", init_main, argument, 8192);
    clock_set_handler(Scheduler::onTickTimer, &Scheduler::instance());
}

uint32_t scheduler_set_timeout(int timeout_ms, timeout_callback_t callback, void* arg) {
    return SystemTimer::instance().addTimer(timeout_ms, (SystemTimer::TimeoutCallbackFunc)callback, arg);
}

void scheduler_unset_timeout(uint32_t timeoutId) {
    SystemTimer::instance().removeTimer(timeoutId);
}

pid_t scheduler_current_pid(void) {
    return Scheduler::instance().getCurrentPid();
}

pid_t scheduler_current_tid(void) {
    return Scheduler::instance().getCurrentTid();
}

}

int scheduler_syscall_handler(struct user_regs **regs, const struct kernel_syscall* syscall, va_list args) {
    return Scheduler::instance().syscall_entry_point(regs, syscall, args);
}

// extern "C"
