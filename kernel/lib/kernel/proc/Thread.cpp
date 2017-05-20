#include <cstring>
#include <cassert>
#include <platform/process.h>
#include <lib/mm/physmem.h>
#include <platform/panic.h>
#include <lib/primitives/align.h>
#include "lib/kernel/proc/Thread.h"
#include "lib/kernel/IdAllocator.h"
#include "lib/kernel/proc/Scheduler.h"
#include "lib/kernel/proc/SystemTimer.h"
#include "signals.h"

static IdAllocator gTidAllocator(PID_THREAD_START, PID_THREAD_END);

Thread::Thread(Process& process,
               const char *name,
               EntryPointFunction entryPoint, void *argument,
               size_t stackSize)

    :  _kernelStackPage(nullptr),
       _platformThreadCb(),
       _preemptionContext(Thread::ContextType::UserSpace, false),
       _schedulingPolicyData(nullptr),
       _state(State::READY),
       _tid(gTidAllocator.allocate()),
       _exitCode(0),
       _cpuTime(0),
       _entryPoint(entryPoint),
       _argument(argument),
       _proc(process),
       _pending_signal_nr(SIG_NONE),
       _userStackRegion(nullptr)
{
    assert(_tid != -1);
    strncpy(_name, name, sizeof(_name));

    if ((stackSize % PAGESIZE) != 0) {
        const auto suggested = align(stackSize, PAGESIZE);
        panic("%s: thread stack size must be aligned to page size (%d): %d. try this: %d\n", name, PAGESIZE, stackSize, suggested);
    }

    _userStackRegion = _proc.allocateStackRegion("stack#" + std::to_string(_tid), stackSize);
    assert(NULL != _userStackRegion);

    _kernelStackPage = pm_alloc(KernelStackSize / PAGESIZE);
    assert(nullptr != _kernelStackPage);

    memset((void*) PG_VADDR_START(_kernelStackPage), 0xef, KernelStackSize);

    if (NULL == _entryPoint) {
        panic("Tried to create a process with NULL as entry point.. shame..");
    }

    int is_kernel_proc = _proc.is_kernel_proc() ? 1 : 0;
    _platformThreadCb.stack_pointer = platform_initialize_stack(
            (void *) PG_VADDR_START(_kernelStackPage), KernelStackSize,
            is_kernel_proc ? nullptr : (char*)(_userStackRegion->startAddress() + stackSize),
            (void*) _entryPoint, _argument,
            (void *) 0x0,
            is_kernel_proc
    );
}

Thread::Thread(const Thread& other, Process& father, const struct user_regs* regs)
    : _kernelStackPage(nullptr),
      _platformThreadCb(),
      _preemptionContext(other._preemptionContext),
      _schedulingPolicyData(nullptr),
      _state(State::READY),
      _tid(gTidAllocator.allocate()),
      _exitCode(0),
      _cpuTime(0),
      _entryPoint(other._entryPoint),
      _argument(other._argument),
      _proc(father),
      _pending_signal_nr(SIG_NONE),
      _userStackRegion(nullptr)
{
    assert(_tid != -1);
    strncpy(_name, other._name, sizeof(_name));

    _userStackRegion = _proc._memoryMap->get(other._userStackRegion->name());
    assert(NULL != _userStackRegion);

    _kernelStackPage = pm_alloc(KernelStackSize / PAGESIZE);
    assert(nullptr != _kernelStackPage);

    memset((void*) PG_VADDR_START(_kernelStackPage), 0xef, KernelStackSize);

    _platformThreadCb.stack_pointer = platform_copy_stack((void *) PG_VADDR_START(_kernelStackPage),
                                                          KernelStackSize,
                                                          regs);
    // this is called from a fork syscall, and we need to skip the syscall opcode
    platform_thread_advance_pc(&_platformThreadCb, 1);
    // this sets the child's retval to 0
    platform_thread_set_return_value(&_platformThreadCb, 0);
}

Thread::~Thread(void) {
    InterruptsMutex guard(true);

    if (_userStackRegion) {
        _proc._memoryMap->destroyMemoryRegion(_userStackRegion->name());
    }

    if (_kernelStackPage) {
        pm_free(_kernelStackPage);
    }

    gTidAllocator.deallocate(_tid);
}

bool Thread::signal(int sig_nr) {
    int oldValue = SIG_NONE;
    return _pending_signal_nr.compare_exchange_weak(oldValue, sig_nr);
}

bool Thread::sleep(int ms) {
    InterruptsMutex mutex(true);

    SystemTimer::instance().addTimer(ms, [](void* arg) {
        Thread* self = (Thread*)arg;
        Scheduler::instance().resume(self, 0);
        return false;
    }, this);

    // now, stop the proc
    return Scheduler::instance().suspend(this);
}

void Thread::yield(void) {
    if (_schedulingPolicyData) {
        _schedulingPolicyData->yieldRequested = true;
    }
}

bool Thread::is_in_kstack_range(void* ptr) {
    const char* start = (char*) PG_VADDR_START(_kernelStackPage);
    const char* end = start + KernelStackSize;
    const char* pos = (char*)ptr;

    return pointer_is_in_range(pos, start, end);
}
