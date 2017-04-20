#include <platform/panic.h>
#include <signals.h>
#include "SignalProvider.h"
#include "proc.h"

SignalProvider& SignalProvider::instance(void) {
    auto* instance = signal_provider_get();

    if (!instance) {
        panic("SignalProvider instance is NULL. yayks..");
    }

    return *instance;
}

SignalProvider::SignalProvider(ProcessProvider& processProvider, Scheduler& scheduler)
    : _processProvider(processProvider),
      _scheduler(scheduler)
{

}

bool SignalProvider::signalPid(pid_t pid, int signal_nr, uintptr_t value) {
    if (PID_CURRENT == pid) {
        pid = _scheduler.getCurrentPid();
    }

    switch (pid) {
        case PID_PROCESS_START ... PID_PROCESS_END:
            return this->signalProcessPid(pid, signal_nr, value);

        case PID_THREAD_START ... PID_THREAD_END:
            return this->signalThreadTid(pid, signal_nr, value);

        default:
            return false;
    }
}

bool SignalProvider::signalProcessPid(pid_t pid, int signal_nr, uintptr_t value) {
    Process* proc = _processProvider.getProcessByPid(pid);
    if (!proc) {
        return false;
    }

    switch (signal_nr)
    {
        case SIG_STOP:
            return _scheduler.suspend(proc);

        case SIG_CONT:
            return _scheduler.resume(proc, value);

        case SIG_ABORT:
        case SIG_KILL:
            return _scheduler.kill(proc, false);

        default:
            kprintf("%s: unknown signal received %d\n", proc->name(), signal_nr);
            return false;
    }
}

bool SignalProvider::signalThreadTid(pid_t tid, int signal_nr, uintptr_t value) {
    Thread* thread = _processProvider.getThreadByTid(tid);
    if (!thread) {
        return false;
    }

    switch (signal_nr)
    {
        case SIG_STOP:
            return _scheduler.suspend(thread);

        case SIG_CONT:
            return _scheduler.resume(thread, value);

        case SIG_ABORT:
        case SIG_KILL:
            return _scheduler.kill(thread);

        default:
            kprintf("%s: unknown signal received %d\n", thread->name(), signal_nr);
            return false;
    }
}
