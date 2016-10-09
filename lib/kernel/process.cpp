#include <lib/kernel/process.h>
#include <cstring>
#include <platform/process.h>
#include <platform/panic.h>
#include <platform/kprintf.h>
#include <lib/kernel/signals.h>
#include <cassert>
#include "scheduler.h"

static std::atomic<pid_t> gNextPid(PID_PROCESS_START);
static pid_t generatePid(void) {
    return gNextPid.fetch_add(1, std::memory_order::memory_order_relaxed);
}

Process::Process(const char* name,
                 const void *buffer, size_t size,
                 std::vector<const char*>&& arguments)
        : Process(name, std::move(arguments))
{
    // TODO: load elf?? :)
}

Process::Process(const char *name, std::vector<const char*>&& arguments)
:      _pid(generatePid()),
      _state(Process::State::READY),
      _exitCode(0),
      _entryPoint(nullptr),
      _arguments(std::move(arguments)),
      _pctx(nullptr),
      _pending_signal_nr((int)SIG_NONE)
{
    strncpy(_name, name, sizeof(_name));

    _pctx = platform_initialize_process_ctx(_pid);
    if (!_pctx) {
        panic("Failed to initialize process ctx");
    }

    kprintf("spawn new proc with pid %d named %s\n", _pid, _name);
}

Process::~Process(void)
{
    try {
        this->_fileDescriptors.close_all();
    } catch (std::exception& ex) {

    }

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

int Process::processMainLoop(void* argument)
{
    Process* self = (Process*)argument;

    assert(nullptr != self->_entryPoint);

    self->_exitCode = 0;
    self->_exitCode = self->_entryPoint((int) self->_arguments.size(), self->_arguments.data());
    self->_state = State::TERMINATED;

    kprintf("proc %d (%s): terminated with exit code %d\n", self->_pid, self->_name, self->_exitCode);

    return self->_exitCode;
}

uint64_t  Process::cpu_time() const {
    // TODO: implement
    return 0;
}



