#include <proccess/process.h>
#include <lib/kernel/signals.h>
#include <cassert>
#include <platform/panic.h>
#include <vfs/VirtualFileSystem.h>
#include "sched/scheduler.h"
#include "IdAllocator.h"
#include "vfs/ConsoleFileDescriptor.h"

static IdAllocator gPidAllocator(PID_PROCESS_START, PID_PROCESS_END);

Process::Process(const char* name,
                 ElfLoader& elfLoader,
                 std::vector<std::string>&& arguments)
        : Process(name, std::move(arguments))
{
    if (!elfLoader.loadSections(_memoryMap)) {
        panic("%s (%d): failed to load elf section", _name, _pid);
    }

    uintptr_t endAddr = elfLoader.getEndAddress();
    if (0 == endAddr) {
        panic("%s (%d): failed to obtain end address", _name, _pid);
    }

    if (!_memoryMap.createMemoryRegion("heap",
                                       endAddr, endAddr + PAGESIZE,
                                       (vm_prot_t)(VM_PROT_READ | VM_PROT_WRITE),
                                       true))
    {
        panic("%s (%d): failed to create heap region", _name, _pid);
    }

    _entryPoint = (Thread::EntryPointFunction) elfLoader.getEntryPoint();
}

Process::Process(const char* name, std::vector<std::string>&& arguments, bool initializeFds)
        : _pid(gPidAllocator.allocate()),
          _state(Process::State::READY),
          _exitCode(0),
          _entryPoint(nullptr),
          _arguments(std::move(arguments)),
          _pending_signal_nr((int)SIG_NONE),
          _memoryMap((asid_t) this->pid()),
          _traceme(false),
          _userArgv(nullptr)
{
    assert(_pid != -1);
    strncpy(_name, name, sizeof(_name));

    this->createStackRegion();
    this->createArgsRegion();

    _REENT_INIT_PTR(&_reent);

    if (initializeFds) {
        // setup stdin, stdout, stderr
        _fileDescriptors.push_filedescriptor(VirtualFileSystem::instance().open("/dev/console", O_RDONLY));
        _fileDescriptors.push_filedescriptor(VirtualFileSystem::instance().open("/dev/console", O_WRONLY));
        _fileDescriptors.push_filedescriptor(VirtualFileSystem::instance().open("/dev/console", O_WRONLY));
    }
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

void Process::terminate(int exit_code) {
    InterruptsMutex mutex(true);
    _exitCode = exit_code;
    _state = Process::State::TERMINATED;
}

bool Process::extendHeap(uintptr_t endAddr){
    auto* heapRegion = _memoryMap.get("heap");
    if (!heapRegion) {
        return false;
    }

    return heapRegion->extend(endAddr);
}

bool Process::traceme(bool state) {
    return (_traceme = state);
}

bool Process::traceme(void) {
    return _traceme;
}

const std::vector<std::string>& Process::arguments(void) const {
    return _arguments;
}

void Process::createStackRegion() {
    if (!_memoryMap.createMemoryRegion("stack", 0x70000000, 0x70a00000, (vm_prot_t)(VM_PROT_READ | VM_PROT_WRITE))) {
        panic("failed to create stack region for %d (%s)", _pid, _name);
    }
}

void Process::createArgsRegion() {
    auto* argsRegion = _memoryMap.createMemoryRegion("args",
                                                     0x70b00000, 0x70b01000,
                                                     (vm_prot_t) (VM_PROT_READ | VM_PROT_WRITE));
    if (!argsRegion) {
        panic("failed to create args region for %d (%s)", _pid, _name);
    }

    auto argv_elements = _arguments.size() + 1; // for null ptr at the end
    auto argv_table_size = (sizeof(const char*) * argv_elements);
    int argv_size = argv_table_size;
    for (const auto& arg : _arguments) {
        argv_size += arg.size() + 1; // plus null
    }

    char* user_argv = (char*) argsRegion->allocate(argv_size);
    if (!user_argv) {
        panic("failed to allocate argv region for %d (%s)", _pid, _name);
    }

    kprintf("[process] user args ptr: %p\n", user_argv);

    _memoryMap.runInScope([this, user_argv, argv_table_size]() {
        char** argv_table = (char**) (user_argv);
        char* argv_data_pos = (user_argv + argv_table_size);
        size_t index;

        for (index = 0; index < this->_arguments.size(); index++) {
            const auto& arg = this->_arguments[index];
            kprintf("arg: %s %d\n", arg.c_str(), arg.size());
            argv_table[index] = argv_data_pos;

            strcpy(argv_data_pos, arg.c_str());
            argv_data_pos += arg.size();
            argv_data_pos += 1; // for the null
        }

        argv_table[index] = nullptr;
    });

    this->_userArgv = user_argv;
    kprintf("[process] user args ptr: %p\n", this->_userArgv);
}