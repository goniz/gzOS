//
// Created by gz on 6/11/16.
//

#include <lib/scheduler/scheduler.h>
#include <platform/panic.h>
#include <cstdio>
#include <platform/kprintf.h>
#include <lib/primitives/queue.h>
#include <platform/clock.h>
#include <lib/primitives/interrupts_mutex.h>
#include <lib/primitives/spinlock_mutex.h>
#include <lib/primitives/lock_guard.h>
#include <platform/process.h>

#define debug_log(msg, ...) if (_debugMode) kprintf(msg "\n", ##__VA_ARGS__)

__attribute__((noreturn))
static int idleProcMain(int argc, const char** argv)
{
    kputs("Idle thread is running!\n");
    while (true) {
        auto clock = clock_get_ms();
        if (clock % 1000) {
            // InterruptGuard guard;
            // kprintf("IdleProc: count %ld\n", clock_get_ms());
			kputs("idleProcMain\n");
        }
    }
}

ProcessScheduler::ProcessScheduler(size_t initialProcSize, size_t initialQueueSize)
    : _currentProc(nullptr),
      _responsiveQueue(initialQueueSize),
      _preemptiveQueue(initialQueueSize),
      _idleProc("IdleProc", idleProcMain, {}, 4096, Process::Type::Preemptive, DefaultPreemptiveQuantum),
      _processList(),
      _mutex()
{
    _processList.reserve(initialProcSize);
}

Process* ProcessScheduler::andTheWinnerIs(void)
{
    Process* newProc = nullptr;

    // first, try getting a task from _responsiveQueue
    if (_responsiveQueue.pop(newProc)) {
        debug_log("andTheWinnerIs: responsive newProc %p name %s", newProc, newProc->_name);
        return newProc;
    }

    // then try getting a task from _preemptiveQueue
    if (_preemptiveQueue.pop(newProc)) {
        debug_log("andTheWinnerIs: preemptive newProc %p name %s", newProc, newProc->_name);
        return newProc;
    }

    debug_log("andTheWinnerIs: idle proc");
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

//static void print_queue(const queue<Process*>& q)
//{
//    kprintf("Queue size: %d var[0]: %p\n", q.size(), q.underlying_data()[0]);
//    for (const auto& var : q.underlying_data())
//    {
//        kprintf("var: %p\n", var);
//    }
//}

// assumes _currentProc is non-null
Process* ProcessScheduler::handlePreemptiveProc(void)
{
    auto currentQuantum = _currentProc->_quantum;

    debug_log("Current quantum: %d", currentQuantum);

    if (_currentProc->_state != Process::State::TERMINATED) {
        // first, check if we're out of quantum
        if (0 == currentQuantum) {
            _currentProc->_state = Process::State::READY;
            _currentProc->_quantum = _currentProc->_resetQuantum;
            _preemptiveQueue.push(_currentProc);
            debug_log("%s: out of quantum, rescheduling\n", _currentProc->_name);
        }
    } else {
        // little hack to stop terminated procs from continuing their run
        _currentProc->_quantum = 0;
        currentQuantum = 0;
    }

    if (currentQuantum > 0) {
        return _currentProc;
    }

    return this->andTheWinnerIs();
}

struct user_regs* ProcessScheduler::schedule(struct user_regs* regs)
{
    lock_guard<spinlock_mutex> guard(_mutex);
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
    platform_set_active_process_ctx(_currentProc->_pctx);
    return _currentProc->_context;
}

// assumes argument is non-null
struct user_regs *ProcessScheduler::onTickTimer(void *argument, struct user_regs *regs)
{
    ProcessScheduler* self = (ProcessScheduler*)argument;

    return self->schedule(regs);
}

//_idleProc("IdleProc", idleProcMain, {}, 1024, Process::Type::Preemptive, DefaultPreemptiveQuantum),
pid_t ProcessScheduler::createPreemptiveProcess(const char *name, Process::EntryPointFunction main,
                                                std::vector<const char*>&& arguments, size_t stackSize, int initialQuantum)
{
    InterruptsMutex mutex;
    mutex.lock();

    std::unique_ptr<Process> newProc = std::make_unique<Process>(name, main, std::move(arguments),
                                                                 stackSize, Process::Type::Preemptive,
                                                                 initialQuantum);

    pid_t newPid = newProc->pid();
    _preemptiveQueue.push(newProc.get());
    _processList.push_back(std::move(newProc));
    return newPid;
}

pid_t ProcessScheduler::createResponsiveProcess(const char *name, Process::EntryPointFunction main,
                                                std::vector<const char*>&& arguments, size_t stackSize, int initialQuantum)
{
    InterruptsMutex mutex;
    mutex.lock();

    std::unique_ptr<Process> newProc = std::make_unique<Process>(name, main, std::move(arguments),
                                                                 stackSize, Process::Type::Responsive,
                                                                 initialQuantum);

    pid_t newPid = newProc->pid();
    _responsiveQueue.push(newProc.get());
    _processList.push_back(std::move(newProc));
    return newPid;
}

void ProcessScheduler::setDebugMode(void)
{
    lock_guard<spinlock_mutex> guard(_mutex);
    _debugMode = true;
}





