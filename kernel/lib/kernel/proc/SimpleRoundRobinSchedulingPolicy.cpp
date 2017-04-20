#include <cassert>
#include "SimpleRoundRobinSchedulingPolicy.h"
#include "Thread.h"
#include "Scheduler.h"

SimpleRoundRobinSchedulingPolicy::SimpleRoundRobinSchedulingPolicy(void)
    : _readyQueue(ReadyQueueCapacity)
{

}

Thread* SimpleRoundRobinSchedulingPolicy::choose(void) {
    Thread* newThread = nullptr;

    if (_readyQueue.pop(newThread, false)) {
        return newThread;
    }

    return nullptr;
}

Thread* SimpleRoundRobinSchedulingPolicy::evaluate_and_choose(Thread* thread) {
    QuantumSchedulingPolicyData& data = thread->schedulingPolicyData();

    // play the quantum card
    if (--data.quantum <= 0) {
        // check if we're out of quantum
        thread->state() = Thread::State::READY;
        data.quantum = data.resetQuantum;
        _readyQueue.push(thread);

        return this->choose();
    }

    return thread;
}

bool SimpleRoundRobinSchedulingPolicy::can_choose(void) const {
    return _readyQueue.size() > 0;
}

void SimpleRoundRobinSchedulingPolicy::suspend(Thread* thread) {
    QuantumSchedulingPolicyData& data = thread->schedulingPolicyData();

    data.quantum = data.resetQuantum;

    thread->state() = Thread::State::SUSPENDED;
}

void SimpleRoundRobinSchedulingPolicy::resume(Thread* thread) {
    QuantumSchedulingPolicyData& data = thread->schedulingPolicyData();

    data.quantum = data.resetQuantum;

    thread->state() = Thread::State::READY;

    // NOTE: take a shortcut to make the system responsive by "cutting the line" when resuming a thread
    Thread* current = Scheduler::instance().CurrentThread();
    if (current) {
        current->schedulingPolicyData().quantum = 0;
    }

    _readyQueue.push_head(thread);
//    _readyQueue.push(thread);
}

bool SimpleRoundRobinSchedulingPolicy::add(Thread* thread) {
    if (!thread) {
        return false;
    }

    InterruptsMutex mutex(true);

    QuantumSchedulingPolicyData& data = thread->schedulingPolicyData();

    data.quantum = data.resetQuantum;

    thread->state() = Thread::State::READY;

    return _readyQueue.push(thread);
}

bool SimpleRoundRobinSchedulingPolicy::remove(Thread* thread) {
    return false;
}
