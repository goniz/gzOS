#include <sched/scheduler.h>
#include <platform/panic.h>
#include <cstdio>
#include <platform/clock.h>
#include <platform/process.h>
#include <lib/kernel/signals.h>
#include <cstring>
#include <platform/drivers.h>
#include <platform/cpu.h>
#include <cassert>
#include <algorithm>
#include "elf/ElfLoader.h"

#define debug_log(msg, ...) if (_debugMode) kprintf(msg "\n", ##__VA_ARGS__)

static Scheduler *gInstance = nullptr;

Scheduler &Scheduler::instance(void) {
    auto *sched = gInstance;

    if (NULL == sched) {
        panic("Scheduler instance is NULL. yayks..");
    }

    return *sched;
}

extern "C" void *scheduler(void) {
    return gInstance;
}

static int scheduler_init(void) {
    gInstance = new Scheduler();
//    gInstance->setDebugMode();
    return 0;
}

DECLARE_DRIVER(scheduler, scheduler_init, STAGE_FIRST);

__attribute__((noreturn))
static int idleProcMain(void *argument) {
    kputs("Idle thread is running!\n");
    while (true) {
        platform_cpu_wait();
        syscall(SYS_NR_YIELD);
    }
}

Scheduler::Scheduler(void)
        : _currentThread(nullptr),
          _readyQueue(SCHED_INITIAL_QUEUE_SIZE),
          _kernelProc(new Process("Kernel", {}, false)),
          _timerProc(new Process("SystemTimer", {}, false)),
          _idleThread(nullptr),
          _processList(),
          _mutex()
{
    _processList.reserve(SCHED_INITIAL_PROC_SIZE);
    _timers.reserve(SCHED_INITIAL_TIMERS_SIZE);

    _processList.push_back(std::unique_ptr<Process>(_kernelProc));

    this->createThread(*_kernelProc, "idle", idleProcMain, nullptr, PAGESIZE);
    // NOTE: hack ahead (because createThread already puts the thread on the ready queue.. sorry..
    _readyQueue.pop(_idleThread, false);
}

Thread *Scheduler::andTheWinnerIs(void) {
    Thread *newThread = nullptr;

    // first, try getting a task from _responsiveQueue
    if (_readyQueue.pop(newThread)) {
        debug_log("andTheWinnerIs: newThread %p name %s", newThread, newThread->_name);
        return newThread;
    }

    debug_log("andTheWinnerIs: idle thread");
    // and if that fails.. get the _idleThread...
    return _idleThread;
}

struct user_regs *Scheduler::schedule(struct user_regs *regs) {
    lock_guard<spinlock_mutex> guard(_mutex);
    // a normal schedule starts here..
    // we've got an existing proc and a potential new one
    if (NULL == _currentThread) {
        // first run, _currentProc is NULL and will be chosen for the first time! YAY!
        _currentThread = this->andTheWinnerIs();
        goto switch_to_proc;
    }

    // save current context
    _currentThread->_platformThreadCb.stack_pointer = regs;
    // advance cpu time counter
    _currentThread->_cpuTime++;

    this->handleSignal(_currentThread);

    if ((_currentThread->_state == Thread::State::TERMINATED) ||
        (_currentThread->_state == Thread::State::SUSPENDED)) {
        _currentThread = this->andTheWinnerIs();
        goto switch_to_proc;
    }

    // play the quantum card
    if (--_currentThread->_quantum <= 0) {
        // check if we're out of quantum
        _currentThread->_state = Thread::State::READY;
        _currentThread->_quantum = _currentThread->_resetQuantum;
        _readyQueue.push(_currentThread);
        debug_log("%s: out of quantum, rescheduling\n", _currentThread->_name);

        _currentThread = this->andTheWinnerIs();
        goto switch_to_proc;
    }


    switch_to_proc:
    Process::switchProcess(_currentThread->proc());
    platform_set_active_thread(&_currentThread->_platformThreadCb);
    // return the context user_regs of the new/existing current proc entry
    _currentThread->_state = Thread::State::RUNNING;
    return _currentThread->_platformThreadCb.stack_pointer;
}

// assumes argument is non-null
struct user_regs *Scheduler::onTickTimer(void *argument, struct user_regs *regs) {
    Scheduler *self = (Scheduler *) argument;

    self->doTimers();

    return self->schedule(regs);
}

Process *Scheduler::createProcess(const char* name,
                                  const void* buffer, size_t size,
                                  std::vector<std::string>&& arguments,
                                  size_t stackSize) {
    ElfLoader loader(buffer, size);
    if (!loader.sanityCheck()) {
        return nullptr;
    }

    auto process = std::make_unique<Process>(name, loader, std::move(arguments));
    if (!process) {
        return nullptr;
    }

    auto mainThread = this->createThread(*process, "main", process->_entryPoint, process->_userArgv, stackSize);
    if (!mainThread) {
        return nullptr;
    }

    kprintf("spawn new proc with pid %d named %s\n", process->pid(), process->name());

    InterruptsMutex mutex(true);
    auto processPtr = process.get();
    _processList.push_back(std::move(process));
    return processPtr;
}

Thread *Scheduler::createThread(Process &process,
                                const char *name,
                                Thread::EntryPointFunction entryPoint, void *argument,
                                size_t stackSize) {
    auto thread = std::make_unique<Thread>(process, name, entryPoint, argument, stackSize, DefaultQuantum);
    auto threadPtr = thread.get();

    kprintf("spawn new thread with tid %d named %s\n", thread->tid(), thread->name());

    InterruptsMutex mutex(true);
    process.threads().push_back(std::move(thread));
    _readyQueue.push(threadPtr);
    return threadPtr;
}

Thread *Scheduler::createKernelThread(const char *name,
                                      Thread::EntryPointFunction entryPoint, void *argument,
                                      size_t stackSize) {
    assert(nullptr != _kernelProc);
    return this->createThread(*_kernelProc, name, entryPoint, argument, stackSize);
}

void Scheduler::setDebugMode(void) {
    _debugMode = true;
}

struct user_regs *Scheduler::yield(struct user_regs *regs) {
    if (!_currentThread) {
        panic("wtf.. yielding with no current proc...");
    }

    if (_readyQueue.empty()) {
        return regs;
    }

    return this->schedule(regs);
}

bool Scheduler::signalPid(pid_t pid, int signal, uintptr_t value) {
    if (PID_CURRENT == pid) {
        pid = getCurrentPid();
    }

    switch (pid) {
        case PID_PROCESS_START ... PID_PROCESS_END:
            return this->signalProcessPid(pid, signal, value);

        case PID_THREAD_START ... PID_THREAD_END:
            return this->signalThreadPid(pid, signal, value);

        default:
            return false;
    }
}

Thread *Scheduler::CurrentThread(void) const {
    return _currentThread;
}

Process *Scheduler::CurrentProcess(void) const {
    Thread *thread = _currentThread;
    if (!thread) {
        return nullptr;
    }

    return &thread->proc();
}

Thread *Scheduler::getThreadByTid(pid_t tid) const {
    if (PID_CURRENT == tid) {
        return _currentThread;
    }

    if (!Scheduler::isThreadPid(tid)) {
        return nullptr;
    }

    InterruptsMutex mutex(true);

    for (const auto &proc : _processList) {
        for (const auto &thread : proc->threads()) {
            if (thread->tid() == tid) {
                return thread.get();
            }
        }
    }

    return nullptr;
}


Process *Scheduler::getProcessByPid(pid_t pid) const {
    InterruptsMutex mutex(true);

    if (PID_CURRENT == pid) {
        return this->CurrentProcess();
    }

    for (const auto &proc : _processList) {
        if (proc->pid() != pid) {
            continue;
        }

        return proc.get();
    }

    return nullptr;
}

pid_t Scheduler::getCurrentPid(void) const {
    Thread *thread = _currentThread;
    if (nullptr == thread) {
        return -1;
    }

    return thread->proc().pid();
}

pid_t Scheduler::getCurrentTid(void) const {
    Thread *thread = _currentThread;
    if (nullptr == thread) {
        return -1;
    }

    return thread->tid();
}

// handle signal as it gets to the running process.
void Scheduler::handleSignal(Thread *thread) {
    const int sig_nr = thread->_pending_signal_nr.exchange(SIG_NONE);

    switch (sig_nr) {
        case SIG_NONE:
            break;

        default:
            kprintf("%s: unknown signal received %d\n", thread->name(), sig_nr);
            break;
    }
}

static uint32_t _id = 1;
uint32_t Scheduler::setTimeout(int timeout_ms, Scheduler::TimeoutCallbackFunc cb, void* arg) {
    InterruptsMutex mutex(true);
    {
        uint64_t now = clock_get_ms();
        auto id = _id++;
        _timers.push_back({id, (uint64_t) timeout_ms, now + timeout_ms, cb, arg});
        return id;
    }
}

void Scheduler::unsetTimeout(uint32_t timerId) {
    InterruptsMutex mutex(true);
    {
        std::remove_if(_timers.begin(), _timers.end(), [timerId](const auto& item) -> bool {
            return item.id == timerId;
        });
    }
}

void Scheduler::doTimers(void) {
    auto iter = _timers.begin();

    while (iter != _timers.end()) {
        bool remove = false;
        uint64_t now = clock_get_ms();
        if (now >= iter->target_ms) {
            if (iter->callbackFunc) {
                _timerProc->switchProcess(*_timerProc);
                remove = !iter->callbackFunc(this, iter->arg);
                if (_currentThread) {
                    _currentThread->proc().switchProcess(_currentThread->proc());
                }

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

void Scheduler::sleep(pid_t pid, int ms) {
    InterruptsMutex mutex(true);

    if (PID_CURRENT == pid) {
        pid = this->getCurrentTid();
    }

    this->setTimeout(ms, [](Scheduler *self, void *arg) {
        self->signalPid((pid_t) arg, SIG_CONT, 0);
        return false;
    }, (void *) pid);

    this->signalPid(pid, SIG_STOP, 0);
}

void Scheduler::suspend(pid_t pid) {
    InterruptsMutex mutex(true);

    this->signalPid(pid, SIG_STOP, 0);
}

void Scheduler::resume(pid_t pid, uintptr_t value) {
    InterruptsMutex mutex(true);

    this->signalPid(pid, SIG_CONT, value);
}

int Scheduler::syscall_entry_point(struct user_regs **regs, const struct kernel_syscall *syscall, va_list args) {
    int ret;

    if (syscall->number != SYS_NR_YIELD && _currentThread && _currentThread->proc().traceme()) {
        // TODO: output this to the proc stderr
        kprintf("traceme %d: entering %s(", _currentThread->proc().pid(), syscall->name);
        vkprintf("%08x, %08x, %08x, %08x, ...)\n", args);
    }

    try {
        switch (syscall->irq) {
            case SYS_IRQ_ENABLED:
                ret = this->handleIRQEnabledSyscall(regs, syscall, args);
                break;

            case SYS_IRQ_DISABLED:
                ret = this->handleIRQDisabledSyscall(regs, syscall, args);
                break;

            default:
                panic("Unknown syscall irq type: %d", syscall->irq);
        }
    } catch (...) {
        ret = -1;
        kprintf("exception occurred in syscall %s: %s\n", syscall->name, "<EX>");
    }

    if (syscall->number != SYS_NR_YIELD && _currentThread && _currentThread->proc().traceme()) {
        // TODO: output this to the proc stderr
        kprintf("traceme %d: leaving %s(", _currentThread->proc().pid(), syscall->name);
        vkprintf("%08x, %08x, %08x, %08x, ...)", args);
        kprintf(" = %d\n", ret);
    }

    return ret;
}

int Scheduler::handleIRQDisabledSyscall(struct user_regs **regs, const kernel_syscall *syscall, va_list args) {
    Thread::PreemptionContext ctx{Thread::ContextType::KernelSpace, true};
    int ret;

    if (_currentThread) {
        std::swap(ctx, _currentThread->_preemptionContext);
    }

    ret = syscall->handler(regs, args);

    if (_currentThread) {
        std::swap(ctx, _currentThread->_preemptionContext);

        const auto state = _currentThread->_state;
        if ((state != Thread::State::READY && state != Thread::State::RUNNING)) {
            *regs = this->schedule(*regs);
        }
    }

    return ret;
}

int Scheduler::handleIRQEnabledSyscall(struct user_regs **regs, const kernel_syscall *syscall, va_list args) {
    int ret = 0;
    Thread::PreemptionContext ctx{Thread::ContextType::KernelSpace, false};
    auto thread = _currentThread;

    if (!thread) {
        panic("IRQ enabled syscalls cannot be called without a running thread. (%d)", syscall->number);
    }

    std::swap(ctx, thread->_preemptionContext);
    auto isrMask = interrupts_enable_save();
    ret = syscall->handler(regs, args);
    interrupts_enable(isrMask);

    std::swap(ctx, thread->_preemptionContext);

    return ret;
}

bool Scheduler::signalThreadPid(pid_t pid, int signal, uintptr_t value) {
    Thread *thread = this->getThreadByTid(pid);
    if (!thread) {
        return false;
    }

    return this->signalThread(thread, signal, value);
}

bool Scheduler::signalThread(Thread* thread, int signal, uintptr_t value) {
    bool ret;
    InterruptsMutex mutex;

    if (Thread::State::TERMINATED == thread->_state) {
        return false;
    }

    // handle signals that need to be taken care of before they get to the running process..
    switch (signal) {
        case SIG_STOP:
            mutex.lock();
            thread->_quantum = thread->_resetQuantum;
            thread->_state = Thread::State::SUSPENDED;

            if (thread == _currentThread && !thread->_preemptionContext.preemptionDisallowed) {
                mutex.unlock();
                syscall(SYS_NR_SCHEDULE);
            }

            ret = true;
            break;

        case Signal::SIG_CONT: {
            mutex.lock();
            thread->_quantum = thread->_resetQuantum;
            thread->_state = Thread::State::READY;
            platform_thread_set_return_value(&thread->_platformThreadCb, value);

            if (thread->_responsive) {
                if (_currentThread) {
                    _currentThread->_quantum = 1;
                }

                _readyQueue.push_head(thread);
            } else {
                _readyQueue.push(thread);
            }

            if (thread == _currentThread && !thread->_preemptionContext.preemptionDisallowed) {
                mutex.unlock();
                syscall(SYS_NR_SCHEDULE);
            }

            ret = true;
            break;
        }

        case Signal::SIG_ABORT:
        case Signal::SIG_KILL:
            mutex.lock();
            thread->_state = Thread::State::TERMINATED;
            thread->_exitCode = 127;

            if (thread == _currentThread && !thread->_preemptionContext.preemptionDisallowed) {
                mutex.unlock();
                syscall(SYS_NR_SCHEDULE);
            }

            ret = true;
            break;

        default:
            ret = thread->signal(signal);
            break;
    }

    return ret;
}

bool Scheduler::signalProcessPid(pid_t pid, int signal, uintptr_t value) {
    Process *proc = this->getProcessByPid(pid);
    if (!proc) {
        return false;
    }

    InterruptsMutex mutex(true);
    for (auto &thread : proc->threads()) {
        auto preemptionDisallowed = true;
        std::swap(thread->_preemptionContext.preemptionDisallowed, preemptionDisallowed);
        this->signalThread(thread.get(), signal, value);
        std::swap(thread->_preemptionContext.preemptionDisallowed, preemptionDisallowed);
    }

    return true;
}

extern "C"
void vm_do_segfault(vm_addr_t fault_addr, vm_prot_t fault_type, vm_prot_t prot) {
    if ((vm_prot_t) -1 == prot) {
        kprintf("Tried to access unmapped memory region: 0x%08x!\n", fault_addr);
    } else if (prot == VM_PROT_NONE) {
        kprintf("Cannot access address: 0x%08x\n", fault_addr);
    } else if (!(prot & VM_PROT_WRITE) && (fault_type == VM_PROT_WRITE)) {
        kprintf("Cannot write to address: 0x%08x\n", fault_addr);
    } else if (!(prot & VM_PROT_READ) && (fault_type == VM_PROT_READ)) {
        kprintf("Cannot read from address: 0x%08x\n", fault_addr);
    }

    auto pid = scheduler_current_pid();
    if (-1 == pid) {
        panic("segfault in kernel..");
    } else {
        kprintf("Killing pid %d\n", pid);
        syscall(SYS_NR_SIGNAL, PID_CURRENT, SIG_KILL);
    }
}

extern "C"
pid_t gettid(void) {
    return syscall(SYS_NR_GET_TID);
}
