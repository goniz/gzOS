#include <cstring>
#include <platform/process.h>
#include <lib/mm/physmem.h>
#include <cassert>
#include <platform/panic.h>
#include <lib/primitives/align.h>
#include "thread.h"
#include "signals.h"
#include "scheduler.h"

static std::atomic<pid_t> gNextTid(PID_THREAD_START);
static pid_t generateTid(void) {
    return gNextTid.fetch_add(1, std::memory_order::memory_order_relaxed);
}

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
       _tid(generateTid()),
       _exitCode(0),
       _cpuTime(0),
       _entryPoint(entryPoint),
       _argument(argument),
       _proc(process),
       _pending_signal_nr(SIG_NONE)
{
    if ((stackSize % PAGESIZE) != 0) {
        auto suggested = align(stackSize, PAGESIZE);
        panic("%s: thread stack size must be aligned to page size (%d): %d. try this: %d\n", name, PAGESIZE, stackSize, suggested);
    }

    _stackPage = pm_alloc(stackSize / PAGESIZE);
    assert(nullptr != _stackPage);

    _context = platform_initialize_stack(
            (void *) PG_VADDR_START(_stackPage), PG_SIZE(_stackPage),
            (void *) Thread::threadMainLoop, this,
            nullptr
    );

    strncpy(_name, name, sizeof(_name));

    kprintf("spawn new thread with tid %d named %s\n", _tid, _name);
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