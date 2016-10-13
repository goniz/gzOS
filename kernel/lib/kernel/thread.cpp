#include <cstring>
#include <platform/process.h>
#include <lib/mm/physmem.h>
#include <cassert>
#include <platform/panic.h>
#include <lib/primitives/align.h>
#include "thread.h"
#include "signals.h"
#include "scheduler.h"
#include "IdAllocator.h"

static IdAllocator gTidAllocator(PID_THREAD_START, PID_THREAD_END);

Thread::Thread(Process& process,
               const char *name,
               EntryPointFunction entryPoint, void *argument,
               size_t stackSize, int initialQuantum)

    :  _kernelStackPage(nullptr),
       _context(nullptr),
       _preemptionContext(Thread::ContextType::UserSpace, false),
       _quantum(initialQuantum),
       _resetQuantum(initialQuantum),
       _state(Thread::READY),
       _responsive(false),
       _tid(gTidAllocator.allocate()),
       _exitCode(0),
       _cpuTime(0),
       _entryPoint(entryPoint),
       _argument(argument),
       _proc(process),
       _pending_signal_nr(SIG_NONE),
       _stackHead(nullptr),
       _stackSize(stackSize)
{
    assert(_tid != -1);
    strncpy(_name, name, sizeof(_name));

    if ((stackSize % PAGESIZE) != 0) {
        const auto suggested = align(stackSize, PAGESIZE);
        panic("%s: thread stack size must be aligned to page size (%d): %d. try this: %d\n", name, PAGESIZE, stackSize, suggested);
    }

    auto* stackRegion = _proc._memoryMap.get("stack");
    assert(NULL != stackRegion);

    _stackHead = stackRegion->allocate(_stackSize);
    assert(NULL != _stackHead);

    _kernelStackPage = pm_alloc(KernelStackSize / PAGESIZE);
    assert(nullptr != _kernelStackPage);

    if (NULL == _entryPoint) {
        panic("Tried to create a process with NULL as entry point.. shame..");
    }

    int is_kernel_proc = _proc.is_kernel_proc() ? 1 : 0;
    _context = platform_initialize_stack(
            (void *) PG_VADDR_START(_kernelStackPage), KernelStackSize,
            is_kernel_proc ? nullptr : (char*)_stackHead + _stackSize,
            (void*) _entryPoint, _argument,
            (void *) 0x1337,
            is_kernel_proc
    );
}

Thread::~Thread(void) {
    auto* stackRegion = _proc._memoryMap.get("stack");
    assert(NULL != stackRegion);

    stackRegion->free(_stackHead);
    _stackHead = NULL;

    if (_kernelStackPage) {
        pm_free(_kernelStackPage);
    }

    gTidAllocator.deallocate(_tid);
}

bool Thread::signal(int sig_nr) {
    int oldValue = SIG_NONE;
    return _pending_signal_nr.compare_exchange_weak(oldValue, sig_nr);

}
