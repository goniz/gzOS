#include "lib/kernel/proc/MultiLevelFeedbackQueuePolicy.h"
#include <lib/kernel/proc/Thread.h>
#include "lib/kernel/proc/Scheduler.h"

MultiLevelFeedbackQueuePolicy::MultiLevelFeedbackQueuePolicy(void)
{
    for (int i = 0; i < RunLevels; ++i) {
        _runLevels[i].index = i;
        _runLevels[i].quantum = DefaultQuantum + (20 * i);
        _runLevels[i].queue.reserve(RunLevelDepth);
    }
}

bool MultiLevelFeedbackQueuePolicy::add(Thread* thread) {
    if (!thread) {
        return false;
    }

    auto data = std::make_unique<MultiLevelFeedbackQueuePolicyData>();
    auto& runLevel = _runLevels[0];

    data->quantum = runLevel.quantum;
    data->resetQuantum = runLevel.quantum;
    data->runLevel = runLevel.index;

    InterruptsMutex mutex(true);

    thread->setSchedulerData(std::move(data));

    // NOTE: take a shortcut to make the system responsive by "cutting the line"
    // when adding a thread "high priority" thread
    Thread* current = Scheduler::instance().CurrentThread();
    if (current) {
        current->yield();
    }

    thread->state() = Thread::State::READY;

    return runLevel.queue.push(thread);
}

bool MultiLevelFeedbackQueuePolicy::remove(Thread* thread) {
    InterruptsMutex mutex(true);

    auto& data = this->dataFromThread(thread);
    if (0 > data.runLevel) {
        return false;
    }

    _runLevels[data.runLevel].queue.remove([thread](const auto& item) {
        return item == thread;
    });

    data.runLevel = -1;

    return true;
}

Thread* MultiLevelFeedbackQueuePolicy::choose(void) {
    for (auto& runLevel : _runLevels) {
        Thread* newThread(nullptr);
        if (runLevel.queue.pop(newThread, false)) {
            return newThread;
        }
    }

    return nullptr;
}

Thread* MultiLevelFeedbackQueuePolicy::yield(Thread* thread) {
    auto& data = this->dataFromThread(thread);

    data.quantum = data.resetQuantum;
    data.yieldRequested = false;
    thread->state() = Thread::State::READY;

    _runLevels[data.runLevel].queue.push(thread);

    return this->choose();
}

Thread* MultiLevelFeedbackQueuePolicy::evaluate_and_choose(Thread* thread) {
    auto& data = this->dataFromThread(thread);

    data.quantum -= 1;

    if (data.yieldRequested) {
        return this->yield(thread);
    }

    if (0 >= data.quantum) {

        auto newRunLevel = (data.runLevel >= RunLevels - 1) ? data.runLevel : (data.runLevel + 1);

        if (newRunLevel != data.runLevel) {
            kprintf("thread %s bumped from %d to %d! (quantum empty)\n", thread->name(), data.runLevel, newRunLevel);
        }

        data.runLevel = newRunLevel;
        auto& runLevel = _runLevels[data.runLevel];

        data.quantum = runLevel.quantum;
        data.resetQuantum = runLevel.quantum;
        thread->state() = Thread::State::READY;

        runLevel.queue.push(thread);

        return this->choose();
    }

    return thread;
}

void MultiLevelFeedbackQueuePolicy::suspend(Thread* thread) {
    auto& data = this->dataFromThread(thread);
    auto& runLevel = _runLevels[data.runLevel];

    thread->state() = Thread::State::SUSPENDED;

    runLevel.queue.remove([thread](const auto& item) {
        return item == thread;
    });
}

void MultiLevelFeedbackQueuePolicy::resume(Thread* thread) {
    auto& data = this->dataFromThread(thread);

    if (0 < data.runLevel) {
        data.runLevel--;
        kprintf("thread %s bumped from %d to %d (resumed)!\n", thread->name(), data.runLevel + 1, data.runLevel);
    }

    auto& runLevel = _runLevels[data.runLevel];

    data.quantum = runLevel.quantum;
    data.resetQuantum = runLevel.quantum;
    thread->state() = Thread::State::READY;

    Thread* current = Scheduler::instance().CurrentThread();
    if (current && current->schedulerData()) {
        if (this->dataFromThread(current).runLevel > data.runLevel) {
            current->yield();
        }
    }

    runLevel.queue.push(thread);
}
