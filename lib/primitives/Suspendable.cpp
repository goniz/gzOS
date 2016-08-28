#include <unistd.h>
#include <algorithm>
#include <lib/scheduler/signals.h>
#include <lib/syscall/syscall.h>
#include <lib/scheduler/scheduler.h>
#include "Suspendable.h"
#include "lock_guard.h"

extern "C" int kill(int pid, int sig);

Suspendable::Suspendable(void)
{
    // this is an optimization, we allocate 2 elements because 99.9% of cases
    // will contain only 1 waiting pid +1 for good luck
    m_waitingPids.reserve(2);
}

Suspendable::~Suspendable(void)
{
    // TODO: this is shit, replace this with a proper cleanup
    // make sure that processes that are still waiting will exit nicely
    // instead of returning from suspended state into an garbage object
    this->notifyAll();
}

void Suspendable::wait(void)
{
    if (platform_is_irq_context()) {
        kprintf("Cannot sleep/wait in irq context..!\n");
        return;
    }

    pid_t currentPid = getpid();

    {
        lock_guard<InterruptsMutex> guard(m_mutex);
        if (m_waitingPids.end() == std::find(m_waitingPids.begin(), m_waitingPids.end(), currentPid)) {

            m_waitingPids.push_back(currentPid);
        }
    }

    kill(currentPid, SIG_STOP);
    syscall(SYS_NR_YIELD);
}

static void signal(pid_t pid, int signal) {
    if (platform_is_irq_context()) {
        scheduler()->signalProc(pid, signal);
    } else {
        kill(pid, signal);
    }
}

void Suspendable::notifyOne(void)
{
    lock_guard<InterruptsMutex> guard(m_mutex);

    if (m_waitingPids.empty()) {
        return;;
    }

    pid_t pid = m_waitingPids.back();
    m_waitingPids.pop_back();


    signal(pid, SIG_CONT);

}

void Suspendable::notifyAll(void)
{
    lock_guard<InterruptsMutex> guard(m_mutex);

    for (pid_t pid : m_waitingPids) {
        signal(pid, SIG_CONT);
    }

    m_waitingPids.clear();
}
