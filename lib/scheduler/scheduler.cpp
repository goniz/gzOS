//
// Created by gz on 6/11/16.
//

#include <lib/scheduler/scheduler.h>
#include <platform/panic.h>
#include <cstdio>
#include <platform/kprintf.h>
#include <lib/primitives/queue.h>
#include <platform/clock.h>

__attribute__((noreturn))
static int idleProcMain(int argc, const char** argv)
{
    printf("Idle thread is running!\n");
    while (true) {
        auto clock = clock_get_ms();
        if (clock % 1000) {
            printf("IdleProc: count %ld\n", clock_get_ms());
        }
    }
}

ProcessScheduler::ProcessScheduler(size_t initialProcSize, size_t initialQueueSize)
    : _currentProc(nullptr),
      _responsiveQueue(initialQueueSize),
      _preemptiveQueue(initialQueueSize),
      _idleProc("IdleProc", idleProcMain, {}, 1024, Process::Type::Preemptive, DefaultPreemptiveQuantum),
      _processList()
{
    _processList.reserve(initialProcSize);
}

Process *ProcessScheduler::andTheWinnerIs(void)
{
    Process* newProc;

    // first, try getting a task from _responsiveQueue
    if (_responsiveQueue.pop(newProc)) {
        return newProc;
    }

    // then try getting a task from _preemptiveQueue
    if (_preemptiveQueue.pop(newProc)) {
        return newProc;
    }

    // and if that fails.. get the _idleProc...
    return &_idleProc;
}

// assumes _currentProc is non-null
Process* ProcessScheduler::handleResponsiveProc(void)
{
    // if the process is a responsive one, keep it running until he yield()s
    // but if he passes the intentionally-very-long-quantum value, kill it.
    if (0 == _currentProc->_quantum) {
        _currentProc->_exitCode = -127;
        _currentProc->_state = Process::State::TERMINATED;
        return this->andTheWinnerIs();
    }

    return _currentProc;
}

static void print_queue(const queue<Process*>& q)
{
    kprintf("Queue size: %d var[0]: %p\n", q.size(), q.underlying_data()[0]);
    for (const auto& var : q.underlying_data())
    {
        kprintf("var: %p\n", var);
    }
}

// assumes _currentProc is non-null
Process* ProcessScheduler::handlePreemptiveProc(void)
{
    Process* newProc = nullptr;
    const auto currentQuantum = _currentProc->_quantum;

    // first, check if we're out of quantum
    if (0 == _currentProc->_quantum) {
        _currentProc->_quantum = DefaultPreemptiveQuantum;
        _preemptiveQueue.push(_currentProc);
        kprintf("%s: out of quantum, rescheduling\n", _currentProc->_name);
        print_queue(_preemptiveQueue);
    }

    // then we give _responsiveQueue a chance before we continue
    if (_responsiveQueue.pop(newProc)) {
        kprintf("%s: yielding in favor of responsive task %s\n", _currentProc->_name, newProc->_name);
        return newProc;
    }

    if (currentQuantum > 0) {
        // now, if we still got quantum, keep running (yes, double check..)
        return _currentProc;
    } else {
        kprintf("%s: yielding in favor of preemptive task %s\n", _currentProc->_name, "");
        print_queue(_preemptiveQueue);
        // but if we dont, pop another preemptive task
        // we dont check for pop() retval because we just pushed to _preemptiveQueue
        auto ret = _preemptiveQueue.pop(newProc);
        kprintf("%s: after pop newProc %p retval %s\n", _currentProc->_name, newProc, ret ? "true" : "false");
        return newProc;
    }
}

struct user_regs* ProcessScheduler::schedule(struct user_regs* regs)
{
    // a normal schedule starts here..
    // we've got an existing proc and a potential new one
    if (NULL != _currentProc)
    {
        // save current context
        _currentProc->_context = regs;
        // play the quantum card
        _currentProc->_quantum--;

        // handle the actual switching logic
        switch (_currentProc->_type)
        {
            case Process::Responsive:
                _currentProc = this->handleResponsiveProc();
                break;

            case Process::Preemptive:
                _currentProc = this->handlePreemptiveProc();
                break;

            default:
                panic("%s: Unknown proc type %d", _currentProc->_name, _currentProc->_type);
        }
    }
    else
    {
        // first run, _currentProc is NULL and will be chosen for the first time! YAY!
        _currentProc = this->andTheWinnerIs();
    }

    // return the context user_regs of the new/existing current proc entry
    _currentProc->_state = Process::State::RUNNING;
    return _currentProc->_context;
}

// assumes argument is non-null
struct user_regs *ProcessScheduler::onTickTimer(void *argument, struct user_regs *regs)
{
    ProcessScheduler* self = (ProcessScheduler*)argument;

    return self->schedule(regs);
}
