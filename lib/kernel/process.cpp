//
// Created by gz on 6/12/16.
//


#include <lib/kernel/process.h>
#include <cstring>
#include <platform/process.h>
#include <platform/panic.h>
#include <platform/kprintf.h>
#include <lib/kernel/signals.h>

static std::atomic<pid_t> g_next_pid(1);
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
      _cpuTime(0),
      _entryPoint(entryPoint),
      _arguments(std::move(arguments)),
      _pending_signal_nr((int)SIG_NONE)
{
    strncpy(_name, name, sizeof(_name));

    struct process_entry_info info{Process::processMainLoop, this};
    _pctx = platform_initialize_process_ctx(_pid, stackSize);
    if (!_pctx) {
        panic("Failed to initialize process ctx");
    }

    _context = platform_initialize_process_stack(_pctx, &info);
    kprintf("spawn new proc with pid %d named %s\n", _pid, _name);
}


Process::~Process(void)
{
    platform_free_process_ctx(_pctx);
}

bool Process::signal(int sig_nr)
{
    int oldValue = SIG_NONE;
    return _pending_signal_nr.compare_exchange_weak(oldValue, sig_nr);
}

const char *Process::name(void) const
{
    return _name;
}

int Process::exit_code(void) const
{
    return _exitCode;
}

int Process::state(void) const
{
    return (int)_state;
}

int Process::type(void) const
{
    return (int)_type;
}

uint64_t Process::cpu_time(void) const {
    return _cpuTime;
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



