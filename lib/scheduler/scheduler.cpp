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
#include <lib/syscall/syscall.h>

#define debug_log(msg, ...) if (_debugMode) kprintf(msg "\n", ##__VA_ARGS__)

static std::unique_ptr<ProcessScheduler> global_scheduler;

extern "C"
void scheduler_init(size_t initialProcSize, size_t initialQueueSize, init_main_t init_main, int argc, const char** argv)
{
    global_scheduler = std::make_unique<ProcessScheduler>(initialProcSize, initialQueueSize);

//    global_scheduler->setDebugMode();
    std::vector<const char*> arguments(argv, argv + argc);
    global_scheduler->createPreemptiveProcess("init", init_main, std::move(arguments), 4096);
//    syscall(0, "init", init_main, argc, argv, 4096);

    clock_set_handler(ProcessScheduler::onTickTimer, global_scheduler.get());
    interrupts_enable_all();
}

__attribute__((noreturn))
static int idleProcMain(int argc, const char** argv)
{
    kputs("Idle thread is running!\n");
    while (true);
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
    if (_currentProc->_state == Process::State::YIELDING) {
        // choose a new proc before pushing current proc to responsive queue
        Process* newProc = this->andTheWinnerIs();

        // reset current proc and push to queue
        _currentProc->_quantum = _currentProc->_resetQuantum;
        _currentProc->_state = Process::State::READY;

        // if we picked the idle proc, it means we've got no one else to run but us.
        // then keep running
        if (newProc == &_idleProc) {
            newProc = _currentProc;
        } else {
            // but if we found some one else to actually run, queue us for later
            _responsiveQueue.push(_currentProc);
        }

        return newProc;
    }

    // if the process is a responsive one, keep it running until he yield()s
    // but if he passes the intentionally-very-long-quantum value, kill it.
    if (0 >= _currentProc->_quantum) {
        _currentProc->_exitCode = -127;
        _currentProc->_state = Process::State::TERMINATED;
        return this->andTheWinnerIs();
    }

    return _currentProc;
}

// assumes _currentProc is non-null
Process* ProcessScheduler::handlePreemptiveProc(void)
{
    auto currentQuantum = _currentProc->_quantum;

    debug_log("Current quantum: %d", currentQuantum);

    // little hack to stop terminated procs from continuing their run
    if (_currentProc->_state == Process::State::TERMINATED) {
        return this->andTheWinnerIs();
    }

    if (currentQuantum > 0) {
        return _currentProc;
    } else {
        // check if we're out of quantum
        _currentProc->_state = Process::State::READY;
        _currentProc->_quantum = _currentProc->_resetQuantum;
        _preemptiveQueue.push(_currentProc);
        debug_log("%s: out of quantum, rescheduling\n", _currentProc->_name);
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

struct user_regs *ProcessScheduler::yield(struct user_regs *regs)
{
    if (!_currentProc) {
        panic("wtf.. yielding with no current proc...");
    }

    _currentProc->_state = Process::State::YIELDING;
    return this->schedule(regs);
}


DEFINE_SYSCALL(SYS_NR_CREATE_PREEMPTIVE_PROC, create_preemptive_process)
{
    SYSCALL_ARG(const char*, name);
    SYSCALL_ARG(Process::EntryPointFunction, main);
    SYSCALL_ARG(int, argc);
    SYSCALL_ARG(const char**, argv);
    SYSCALL_ARG(size_t, stackSize);

    std::vector<const char*> arguments(argv, argv + argc);
    return global_scheduler->createPreemptiveProcess(name, main, std::move(arguments), stackSize);
}

DEFINE_SYSCALL(SYS_NR_CREATE_RESPONSIVE_PROC, create_responsive_process)
{
    SYSCALL_ARG(const char*, name);
    SYSCALL_ARG(Process::EntryPointFunction, main);
    SYSCALL_ARG(int, argc);
    SYSCALL_ARG(const char**, argv);
    SYSCALL_ARG(size_t, stackSize);

    std::vector<const char*> arguments(argv, argv + argc);
    return global_scheduler->createResponsiveProcess(name, main, std::move(arguments), stackSize);
}

DEFINE_SYSCALL(SYS_NR_GET_PID, get_pid)
{
    return global_scheduler->getCurrentPid();
}

DEFINE_SYSCALL(SYS_NR_YIELD, yield)
{
    *regs = global_scheduler->yield(*regs);
    return 0;
}