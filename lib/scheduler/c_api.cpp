
#include <platform/clock.h>
#include "scheduler.h"

extern "C" {

void scheduler_run_main(init_main_t init_main, int argc, const char** argv)
{
    std::vector<const char*> arguments(argv, argv + argc);
    scheduler()->createPreemptiveProcess("init", init_main, std::move(arguments), 8096);
    clock_set_handler(ProcessScheduler::onTickTimer, scheduler());
}

int scheduler_signal_process(pid_t pid, int signal) {
    InterruptsMutex mutex;
    mutex.lock();
    return scheduler()->signalProc(pid, signal);
}

int scheduler_set_timeout(int timeout_ms, timeout_callback_t callback, void* arg) {
    return scheduler()->setTimeout(timeout_ms, (ProcessScheduler::TimeoutCallbackFunc) callback, arg);
}

void scheduler_sleep(int timeout_ms) {
    const auto pid = scheduler()->getCurrentPid();
    scheduler()->sleep(pid, timeout_ms);
}

void scheduler_suspend(void) {
    const auto pid = scheduler()->getCurrentPid();
    scheduler()->suspend(pid);
}

void scheduler_resume(pid_t pid) {
    scheduler()->resume(pid);
}

pid_t scheduler_current_pid(void)
{
    return scheduler()->getCurrentPid();
}

}