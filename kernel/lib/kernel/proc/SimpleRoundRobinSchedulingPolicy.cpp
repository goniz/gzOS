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
    auto& data = this->dataFromThread(thread);

    // play the quantum card
    if (data.yieldRequested || --data.quantum <= 0) {
        // check if we're out of quantum
        return this->yield(thread);
    }

    return thread;
}

void SimpleRoundRobinSchedulingPolicy::suspend(Thread* thread) {
    auto& data = this->dataFromThread(thread);

    data.quantum = data.resetQuantum;

    thread->state() = Thread::State::SUSPENDED;
}

void SimpleRoundRobinSchedulingPolicy::resume(Thread* thread) {
    auto& data = this->dataFromThread(thread);

    data.quantum = data.resetQuantum;

    thread->state() = Thread::State::READY;

    // NOTE: take a shortcut to make the system responsive by "cutting the line" when resuming a thread
    Thread* current = Scheduler::instance().CurrentThread();
    if (current) {
        this->dataFromThread(thread).quantum = 0;
    }

    _readyQueue.push_head(thread);
//    _readyQueue.push(thread);
}

bool SimpleRoundRobinSchedulingPolicy::add(Thread* thread) {
    if (!thread) {
        return false;
    }

    auto data = std::make_unique<QuantumSchedulingPolicyData>();

    data->quantum = DefaultQuantum;
    data->resetQuantum = DefaultQuantum;

    InterruptsMutex mutex(true);

    thread->setSchedulerData(std::move(data));

    thread->state() = Thread::State::READY;

    return _readyQueue.push(thread);
}

bool SimpleRoundRobinSchedulingPolicy::remove(Thread* thread) {
    return false;
}

Thread* SimpleRoundRobinSchedulingPolicy::yield(Thread* thread) {
    auto& data = this->dataFromThread(thread);

    thread->state() = Thread::State::READY;
    data.quantum = data.resetQuantum;
    data.yieldRequested = false;
    _readyQueue.push(thread);

    return this->choose();
}
