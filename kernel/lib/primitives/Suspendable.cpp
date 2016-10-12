#include <unistd.h>
#include <algorithm>
#include <lib/kernel/signals.h>
#include <lib/syscall/syscall.h>
#include <lib/kernel/scheduler.h>
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
    pid_t currentTid = gettid();

    {
        lock_guard<InterruptsMutex> guard(m_mutex);
        if (m_waitingPids.end() == std::find(m_waitingPids.begin(), m_waitingPids.end(), currentTid)) {

            m_waitingPids.push_back(currentTid);
        }
    }

    kill(currentTid, SIG_STOP);
}

void Suspendable::notifyOne(void)
{
    lock_guard<InterruptsMutex> guard(m_mutex);

    if (m_waitingPids.empty()) {
        return;;
    }

    pid_t pid = m_waitingPids.back();
    m_waitingPids.pop_back();

    scheduler_resume(pid);
}

void Suspendable::notifyAll(void)
{
    lock_guard<InterruptsMutex> guard(m_mutex);

    for (pid_t pid : m_waitingPids) {
        scheduler_resume(pid);
    }

    m_waitingPids.clear();
}