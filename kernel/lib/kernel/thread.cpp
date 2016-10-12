#include <cstring>
#include <platform/process.h>
#include <lib/mm/physmem.h>
#include <cassert>
#include <platform/panic.h>
#include <lib/primitives/align.h>
#include <lib/mm/vm_object.h>
#include <lib/mm/vm_pager.h>
#include "thread.h"
#include "signals.h"
#include "scheduler.h"
#include "IdAllocator.h"

static IdAllocator gTidAllocator(PID_THREAD_START, PID_THREAD_END);

Thread::Thread(Process& process,
               const char *name,
               EntryPointFunction entryPoint, void *argument,
               size_t stackSize, int initialQuantum)

    :  _context(nullptr),
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

    if ((stackSize % PAGESIZE) != 0) {
        const auto suggested = align(stackSize, PAGESIZE);
        panic("%s: thread stack size must be aligned to page size (%d): %d. try this: %d\n", name, PAGESIZE, stackSize, suggested);
    }

    auto* stackRegion = _proc._memoryMap.get("stack");
    assert(NULL != stackRegion);

    _stackHead = stackRegion->allocate(_stackSize);
    assert(NULL != _stackHead);

    _proc._memoryMap.runInScope([&]() {
        _context = platform_initialize_stack(
                _stackHead, _stackSize,
                (void *) Thread::threadMainLoop, this,
                nullptr
        );
    });

    strncpy(_name, name, sizeof(_name));
}

Thread::~Thread(void) {
    auto* stackRegion = _proc._memoryMap.get("stack");
    assert(NULL != stackRegion);

    stackRegion->free(_stackHead);
    _stackHead = NULL;

    gTidAllocator.deallocate(_tid);
}

__attribute__((noreturn))
void Thread::threadMainLoop(void* argument)
{
    Thread* self = (Thread*)argument;

    self->_exitCode = 0;
    self->_exitCode = self->_entryPoint(self->_argument);
    self->_state = State::TERMINATED;
    kprintf("thread %d (%s): terminated with exit code %d\n", self->_tid, self->_name, self->_exitCode);

    while (true);
}

bool Thread::signal(int sig_nr) {
    int oldValue = SIG_NONE;
    return _pending_signal_nr.compare_exchange_weak(oldValue, sig_nr);

}
