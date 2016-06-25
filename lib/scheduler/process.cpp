//
// Created by gz on 6/12/16.
//


#include <lib/scheduler/process.h>
#include <cstring>
#include <platform/process.h>
#include <atomic>
#include <platform/panic.h>
#include <platform/kprintf.h>

static std::atomic<pid_t> g_next_pid(0);
static pid_t generate_pid(void)
{
    return g_next_pid.fetch_add(1, std::memory_order::memory_order_relaxed);
}

Process::Process(const char *name,
                 EntryPointFunction entryPoint, std::vector<const char*>&& arguments,
                 size_t stackSize,
                 enum Type procType, int initialQuantum)

    : _context(nullptr),
      _quantum(initialQuantum),
      _resetQuantum(initialQuantum),
      _state(State::READY),
      _pid(generate_pid()),
      _type(procType),
      _exitCode(0),
      _entryPoint(entryPoint),
      _arguments(std::move(arguments))
{
    strncpy(_name, name, sizeof(_name));

    struct process_entry_info info{Process::processMainLoop, this};
    _pctx = platform_initialize_process_ctx(_pid, stackSize);
    if (!_pctx) {
        panic("Failed to initialize process ctx");
    }

    _context = platform_initialize_process_stack(_pctx, &info);
}


Process::~Process(void)
{
    platform_free_process_ctx(_pctx);
}

__attribute__((noreturn))
void Process::processMainLoop(void* argument)
{
    Process* self = (Process*)argument;

    self->_exitCode = self->_entryPoint((int) self->_arguments.size(), self->_arguments.data());
    self->_state = State::TERMINATED;
    kprintf("%s: terminated with exit code %d\n", self->_name, self->_exitCode);

    while (true);
}





