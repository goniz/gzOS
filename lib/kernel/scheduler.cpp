#include <lib/kernel/scheduler.h>
#include <platform/panic.h>
#include <cstdio>
#include <platform/clock.h>
#include <platform/process.h>
#include <platform/cpu.h>
#include <lib/syscall/syscall.h>
#include <lib/kernel/signals.h>
#include <cstring>
#include <platform/drivers.h>

#define debug_log(msg, ...) if (_debugMode) kprintf(msg "\n", ##__VA_ARGS__)

static std::unique_ptr<ProcessScheduler> global_scheduler;

static int scheduler_init(void)
{
    global_scheduler = std::make_unique<ProcessScheduler>(SCHED_INITIAL_PROC_SIZE, SCHED_INITIAL_QUEUE_SIZE);
//    global_scheduler->setDebugMode();
    return 0;
}

DECLARE_DRIVER(scheduler, scheduler_init, STAGE_FIRST);

__attribute__((noreturn))
static int idleProcMain(int argc, const char** argv)
{
    kputs("Idle thread is running!\n");
    while (true) {
		platform_cpu_wait();
        syscall(SYS_NR_YIELD);
    }
}

ProcessScheduler::ProcessScheduler(size_t initialProcSize, size_t initialQueueSize)
    : _currentProc(nullptr),
      _responsiveQueue(initialQueueSize),
      _preemptiveQueue(initialQueueSize),
      _idleProc("IdleProc", idleProcMain, {}, 8096, Process::Type::Preemptive, DefaultPreemptiveQuantum),
      _processList(),
      _mutex()
{
    _processList.reserve(initialProcSize);
    _timers.reserve(200);
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

//    debug_log("Current quantum: %d", currentQuantum);

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
        _currentProc->_cpuTime++;

        this->handleSignal(_currentProc);

        if (_currentProc->_state == Process::State::TERMINATED) {
            _currentProc = this->andTheWinnerIs();
            goto switch_to_proc;
        }

        if (_currentProc->_state == Process::State::SUSPENDED) {
            _currentProc = this->andTheWinnerIs();
            goto switch_to_proc;
        }

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

        goto switch_to_proc;
    }
    else
    {
        // first run, _currentProc is NULL and will be chosen for the first time! YAY!
        _currentProc = this->andTheWinnerIs();
        goto switch_to_proc;
    }

switch_to_proc:
    // return the context user_regs of the new/existing current proc entry
    _currentProc->_state = Process::State::RUNNING;
    platform_set_active_process_ctx(_currentProc->_pctx);
    return _currentProc->_context;
}

// assumes argument is non-null
struct user_regs *ProcessScheduler::onTickTimer(void *argument, struct user_regs *regs)
{
    ProcessScheduler* self = (ProcessScheduler*)argument;

    self->doTimers();

    return self->schedule(regs);
}

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

bool ProcessScheduler::signalProc(pid_t pid, int signal)
{
    Process* proc = this->getProcessByPid(pid);
    if (!proc) {
        return false;
    }

    if (Process::State::TERMINATED == proc->_state) {
        return false;
    }

    // handle signals that need to be taken care of before then get to the running process..
    switch (signal)
    {
        case Signal::SIG_CONT:
        {
            proc->_quantum = proc->_resetQuantum;
            proc->_state = Process::State::READY;

            switch (proc->_type) {
                case Process::Responsive:
                    _responsiveQueue.push_head(proc);
                    // this is a fast path for waking up a responsive process
                    // we just cut the quantum of the current process in order to push forward our
                    // newly awaken responsive process forward in line
                    if (_currentProc) {
                        _currentProc->_quantum = 1;
                    }

                    break;
                case Process::Preemptive:
                    _preemptiveQueue.push(proc);
                    break;
            }

            return true;
        }

        default:
            return proc->signal(signal);
    }
}

Process* ProcessScheduler::getProcessByPid(pid_t pid) const
{
    for (const auto& proc : _processList)
    {
        if (proc->pid() != pid) {
            continue;
        }

        return proc.get();
    }

    return nullptr;
}

// handle signal as it gets to the running process.
void ProcessScheduler::handleSignal(Process *proc)
{
    const int sig_nr = proc->_pending_signal_nr.exchange(SIG_NONE);

    switch (sig_nr)
    {
        case SIG_KILL:
            proc->_exitCode = -127;
            proc->_state = Process::State::TERMINATED;
            break;

        case SIG_STOP:
            proc->_state = Process::State::SUSPENDED;
            break;

        case SIG_NONE:
            break;

        default:
            kprintf("%s: unknown signal received %d\n", proc->name(), sig_nr);
            break;
    }
}

bool ProcessScheduler::setTimeout(int timeout_ms, ProcessScheduler::TimeoutCallbackFunc cb, void* arg) {
    InterruptsMutex mutex;
    mutex.lock();

    uint64_t now = clock_get_ms();
    _timers.push_back({(uint64_t) timeout_ms, now + timeout_ms, cb, arg});
    return true;
}

void ProcessScheduler::doTimers(void) {
    auto iter = _timers.begin();

    while (iter != _timers.end()) {
        bool remove = false;
        uint64_t  now = clock_get_ms();
        if (now >= iter->target_ms) {
            if (iter->callbackFunc) {
                remove = !iter->callbackFunc(this, iter->arg);
                if (!remove) {
                    iter->target_ms = now + iter->timeout_ms;
                }
            }
        }

        if (remove) {
            iter = _timers.erase(iter);
        } else {
            ++iter;
        }
    }
}

void ProcessScheduler::sleep(pid_t pid, int ms) {
    InterruptsMutex mutex;
    mutex.lock();

    this->signalProc(pid, SIG_STOP);

    this->setTimeout(ms, [](ProcessScheduler* self, void* arg) {
        self->signalProc((pid_t) arg, SIG_CONT);
        return false;
    }, (void *) pid);

    mutex.unlock();

    syscall(SYS_NR_YIELD);
}

void ProcessScheduler::suspend(pid_t pid) {
    InterruptsMutex mutex;

    mutex.lock();
    this->signalProc(pid, SIG_STOP);
    mutex.unlock();

    syscall(SYS_NR_YIELD);
}

void ProcessScheduler::resume(pid_t pid) {
    InterruptsMutex mutex;

    mutex.lock();
    this->signalProc(pid, SIG_CONT);
    mutex.unlock();
}

Process *ProcessScheduler::getCurrentProcess(void) const {
    return _currentProc;
}

ProcessScheduler *scheduler(void) {
    auto* sched = global_scheduler.get();

    if (NULL == sched) {
        panic("global_scheduler is NULL. yayks..");
    }

    return sched;
}
