#include "lib/kernel/proc/proc.h"
#include <proc/Scheduler.h>
#include <platform/panic.h>
#include <platform/clock.h>
#include <lib/kernel/signals.h>
#include <platform/cpu.h>
#include <cassert>
#include <algorithm>

#define debug_log(msg, ...) if (_debugMode) kprintf(msg "\n", ##__VA_ARGS__)

Scheduler& Scheduler::instance(void) {
    auto *sched = scheduler_get();

    if (NULL == sched) {
        panic("Scheduler instance is NULL. yayks..");
    }

    return *sched;
}

__attribute__((noreturn))
static int idleProcMain(void *argument) {
    kputs("Idle thread is running!\n");
    while (true) {
        platform_cpu_wait();
        syscall(SYS_NR_YIELD);
    }
}

Scheduler::Scheduler(ProcessProvider& processProvider, std::unique_ptr<SchedulingPolicy> policy)
        : _currentThread(nullptr),
          _processProvider(processProvider),
          _policy(std::move(policy)),
          _idleThread(nullptr),
          _mutex()
{
    assert(NULL != _policy.get());
}

void Scheduler::initialize(void)
{
    _idleThread = _processProvider.createKernelThread("idle", idleProcMain, nullptr, PAGESIZE, false);
}

struct user_regs* Scheduler::schedule(struct user_regs* regs) {
    lock_guard<spinlock_mutex> guard(_mutex);
    // a normal schedule starts here..
    // we've got an existing proc and a potential new one
    if (NULL == _currentThread) {
        // first run, _currentProc is NULL and will be chosen for the first time! YAY!
        _currentThread = _policy->choose();
        goto switch_to_proc;
    }

    // save current context
    _currentThread->_platformThreadCb.stack_pointer = regs;

    if ((_currentThread->_state == Thread::State::TERMINATED) ||
        (_currentThread->_state == Thread::State::SUSPENDED)) {
        _currentThread = _policy->choose();
        goto switch_to_proc;
    }

    // advance cpu time counter
    _currentThread->_cpuTime++;

    _currentThread = _policy->evaluate_and_choose(_currentThread);

switch_to_proc:
    if (NULL == _currentThread) {
        _currentThread = _idleThread;
    }

    assert(Thread::State::READY == _currentThread->_state ||
           Thread::State::RUNNING == _currentThread->_state);

    _currentThread->proc().activateProcess();
    platform_set_active_thread(&_currentThread->_platformThreadCb);
    // return the context user_regs of the new/existing current proc entry
    _currentThread->_state = Thread::State::RUNNING;
    return _currentThread->_platformThreadCb.stack_pointer;
}

// assumes argument is non-null
struct user_regs *Scheduler::onTickTimer(void *argument, struct user_regs *regs) {
    Scheduler *self = (Scheduler *) argument;

    for (const auto& item : self->_tickHandlers) {
        if (item.function(item.argument, &regs)) {
            return regs;
        }
    }

    return self->schedule(regs);
}

void Scheduler::setDebugMode(void) {
    _debugMode = true;
}

struct user_regs *Scheduler::yield(struct user_regs *regs) {
    if (!_currentThread) {
        panic("wtf.. yielding with no current proc...");
    }

    if (!_policy->can_choose()) {
        return regs;
    }

    return this->schedule(regs);
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

bool Scheduler::suspend(Thread* thread) {
    if (!thread) {
        return false;
    }

    InterruptsMutex mutex(true);

    switch (thread->state())
    {
        case Thread::READY:
        case Thread::RUNNING:

            _policy->suspend(thread);

            if (thread == _currentThread && !thread->_preemptionContext.preemptionDisallowed) {
                mutex.unlock();
                syscall(SYS_NR_SCHEDULE);
            }

            return true;

        case Thread::SUSPENDED:
            return true;

        case Thread::TERMINATED:
            return false;
    }

    return false;
}

bool Scheduler::suspend(Process* process) {
    if (!process) {
        return false;
    }

    InterruptsMutex mutex(true);

    bool should_yield = (_currentThread && &_currentThread->proc() == process);

    for (const auto &thread : process->threads()) {
        auto preemptionDisallowed = true;
        std::swap(thread->_preemptionContext.preemptionDisallowed, preemptionDisallowed);
        this->suspend(thread.get());
        std::swap(thread->_preemptionContext.preemptionDisallowed, preemptionDisallowed);
    }

    if (should_yield) {
        mutex.unlock();
        syscall(SYS_NR_SCHEDULE);
    }

    return true;
}

bool Scheduler::resume(Thread* thread, uintptr_t value) {
    if (!thread) {
        return false;
    }

    if (Thread::SUSPENDED != thread->state()) {
        return false;
    }

    InterruptsMutex mutex(true);

    _policy->resume(thread);

    platform_thread_set_return_value(&thread->_platformThreadCb, value);
    return true;
}

bool Scheduler::resume(Process* process, uintptr_t value) {
    if (!process) {
        return false;
    }

    InterruptsMutex mutex(true);

    for (const auto &thread : process->threads()) {
        auto preemptionDisallowed = true;
        std::swap(thread->_preemptionContext.preemptionDisallowed, preemptionDisallowed);
        this->resume(thread.get(), value);
        std::swap(thread->_preemptionContext.preemptionDisallowed, preemptionDisallowed);
    }

    return true;
}

bool Scheduler::kill(Thread* thread, bool yield) {
    if (!thread) {
        return false;
    }

    if (Thread::TERMINATED == thread->state()) {
        return true;
    }

    InterruptsMutex mutex(true);

    thread->_state = Thread::State::TERMINATED;
    thread->_exitCode = 127;

    if (yield && thread == _currentThread && !thread->_preemptionContext.preemptionDisallowed) {
        mutex.unlock();
        syscall(SYS_NR_SCHEDULE);
    }

    return true;
}

bool Scheduler::kill(Process* process, bool yield) {
    if (!process) {
        kprintf("kill: no proc\n");
        return false;
    }

    InterruptsMutex mutex(true);

    if (process->has_exited()) {
        kprintf("kill: has exited\n");
        return false;
    }

    bool should_yield = (yield && _currentThread && &_currentThread->proc() == process);

    for (const auto &thread : process->threads()) {
        auto preemptionDisallowed = true;
        std::swap(thread->_preemptionContext.preemptionDisallowed, preemptionDisallowed);
        this->kill(thread.get());
        std::swap(thread->_preemptionContext.preemptionDisallowed, preemptionDisallowed);
    }

    if (should_yield) {
        mutex.unlock();
        syscall(SYS_NR_SCHEDULE);
    }

    kprintf("kill: success\n");
    return true;
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

int Scheduler::waitForPid(pid_t pid)
{
    InterruptsMutex mutex(true);
    auto* proc = _processProvider.getProcessByPid(pid);
    if (!proc) {
        return -1;
    }

    if (proc->has_exited()) {
        int exit_code = proc->exit_code();
        this->destroyProcess(proc);
        return exit_code;
    }

    int exit_code = proc->wait_for_exit();
    this->destroyProcess(proc);
    return exit_code;
}

bool Scheduler::destroyProcess(Process* proc) {
    kprintf("[%s] pid %d exited with exit code %d\n", proc->name(), proc->pid(), proc->exit_code());

    if (&_processProvider.SystemProcess() == proc ||
        &SystemTimer::instance().TimerProcess() == proc) {
        return false;
    }

    if (!proc->has_exited()) {
        kprintf("proc is still running!\n");
        return false;
    }

    if (&_currentThread->proc() == proc) {
        kprintf("proc is still current proc!\n");
        _currentThread = nullptr;
    }

    _processProvider.destroyProcess(proc);
    return true;
}

bool Scheduler::addThread(Thread* thread) {
    return _policy->add(thread);
}

bool Scheduler::removeThread(Thread* thread) {
//    InterruptsMutex mutex(true);
    // TODO: implement
    return true;
}

bool Scheduler::addTickHandler(Scheduler::TickCallbackFunc func, void* argument) {
    if (!func) {
        return false;
    }

    InterruptsMutex mutex(true);

    _tickHandlers.push_back(TickControlBlock{
            .function = func,
            .argument = argument
    });

    return true;
}

bool Scheduler::removeTickHandler(Scheduler::TickCallbackFunc func) {
    if (!func) {
        return false;
    }

    InterruptsMutex mutex(true);

    auto remove_iter = std::remove_if(_tickHandlers.begin(), _tickHandlers.end(), [func](const auto& item) -> bool {
        return item.function == func;
    });

    _tickHandlers.erase(remove_iter, _tickHandlers.end());

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
        syscall(SYS_NR_SIGNAL, pid, SIG_KILL);
    }
}
