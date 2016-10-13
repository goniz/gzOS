#include <lib/kernel/process.h>
#include <lib/kernel/signals.h>
#include <cassert>
#include <platform/panic.h>
#include "scheduler.h"
#include "IdAllocator.h"

static IdAllocator gPidAllocator(PID_PROCESS_START, PID_PROCESS_END);

Process::Process(const char* name,
                 ElfLoader& elfLoader,
                 std::vector<const char*>&& arguments)
        : Process(name, std::move(arguments))
{

}

Process::Process(const char *name, std::vector<const char*>&& arguments)
        : _pid(gPidAllocator.allocate()),
          _state(Process::State::READY),
          _exitCode(0),
          _entryPoint(nullptr),
          _arguments(std::move(arguments)),
          _pending_signal_nr((int)SIG_NONE),
          _memoryMap((asid_t) this->pid())
{
    assert(_pid != -1);
    strncpy(_name, name, sizeof(_name));

    if (!_memoryMap.createMemoryRegion("heap",  0x60000000, 0x66400000, (vm_prot_t)(VM_PROT_READ | VM_PROT_WRITE))) {
        panic("failed to create heap region for %d (%s)", _pid, _name);
    }

    if (!_memoryMap.createMemoryRegion("stack", 0x70000000, 0x70a00000, (vm_prot_t)(VM_PROT_READ | VM_PROT_WRITE))) {
        panic("failed to create stack region for %d (%s)", _pid, _name);
    }

    _REENT_INIT_PTR(&_reent);
}

Process::~Process(void)
{
    try {
        this->_fileDescriptors.close_all();
    } catch (std::exception& ex) {

    }

    _reclaim_reent(&_reent);

    gPidAllocator.deallocate(_pid);
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

uint64_t  Process::cpu_time() const
{
    // TODO: implement
    return 0;
}

void Process::switchProcess(Process& newProc)
{
    _REENT = &newProc._reent;
    newProc._memoryMap.activate();
}

bool Process::is_kernel_proc(void) const {
    return 0 == strcmp("Kernel", _name);
}




