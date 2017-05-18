#include <proc/Process.h>
#include <lib/kernel/signals.h>
#include <cassert>
#include <platform/panic.h>
#include <vfs/VirtualFileSystem.h>
#include <lib/primitives/align.h>
#include <workqueue.h>
#include "Scheduler.h"
#include "IdAllocator.h"
#include "vfs/ConsoleFileDescriptor.h"

static IdAllocator gPidAllocator(PID_PROCESS_START, PID_PROCESS_END);

Process::Process(const char* name,
                 ElfLoader& elfLoader,
                 std::vector<std::string>&& arguments)
        : Process(name, std::move(arguments))
{
    if (!elfLoader.loadSections(*_memoryMap)) {
        panic("%s (%d): failed to load elf section", _name, _pid);
    }

    if (!this->createHeapRegion(elfLoader, *_memoryMap)) {
        panic("%s (%d): failed to create heap section", _name, _pid);
    }

    _entryPoint = (Thread::EntryPointFunction) elfLoader.getEntryPoint();
}

Process::Process(const char* name, std::vector<std::string>&& arguments, bool initializeFds)
        : _name(),
          _is_kernel_proc(false),
          _pid(gPidAllocator.allocate()),
          _state(Process::State::READY),
          _exitCode(0),
          _entryPoint(nullptr),
          _arguments(std::move(arguments)),
          _pending_signal_nr((int)SIG_NONE),
          _memoryMap(std::make_unique<ProcessMemoryMap>()),
          _traceme(false),
          _userArgv(nullptr),
          _cwdPath(std::string("/")),
          _cwdNode(nullptr)
{
    assert(_pid != -1);
    strncpy(_name, name, sizeof(_name));

    if (!(_userArgv = this->createArgsRegion(*_memoryMap, _arguments))) {
        panic("failed to create args region for %d (%s)", _pid, _name);
    }

    _REENT_INIT_PTR(&_reent);

    if (initializeFds) {
        // setup stdin, stdout, stderr
        _fileDescriptors.push_filedescriptor(VirtualFileSystem::instance().open("/dev/console", O_RDONLY));
        _fileDescriptors.push_filedescriptor(VirtualFileSystem::instance().open("/dev/console", O_WRONLY));
        _fileDescriptors.push_filedescriptor(VirtualFileSystem::instance().open("/dev/console", O_WRONLY));
    }

    _threads.reserve(5);
}

Process::Process(const Process& father)
    : _is_kernel_proc(father._is_kernel_proc),
      _pid(gPidAllocator.allocate()),
      _state(Process::State::READY),
      _exitCode(0),
      _entryPoint(father._entryPoint),
      _arguments(father._arguments),
      _pending_signal_nr((int)SIG_NONE),
      _memoryMap(std::make_unique<ProcessMemoryMap>(*father._memoryMap)),
      _fileDescriptors(father._fileDescriptors),
      _traceme(father._traceme),
      _userArgv(father._userArgv),
      _cwdPath(father._cwdPath),
      _cwdNode(father._cwdNode)
{
    assert(_pid != -1);
    strncpy(_name, father._name, sizeof(_name));

    _REENT_INIT_PTR(&_reent);

    _threads.reserve(5);
}

bool Process::Exec(const char* name, ElfLoader& elfLoader, std::vector<std::string>&& arguments)
{
    auto newMap = std::make_unique<ProcessMemoryMap>();
    if (!newMap) {
        return false;
    }

    if (!elfLoader.loadSections(*newMap)) {
        kprintf("load sections failed\n");
        return false;
    }

    if (!this->createHeapRegion(elfLoader, *newMap)) {
        kprintf("heap creation failed\n");
        return false;
    }

    char** userArgv = this->createArgsRegion(*newMap, arguments);
    if (!userArgv) {
        kprintf("args creation failed\n");
        return false;
    }

    if (!this->discardAllThreads()) {
        kprintf("thread discard failed\n");
        // panic??
        return false;
    }

    std::swap(_memoryMap, newMap);

    workqueue_put([](void* arg) -> void {
        delete((ProcessMemoryMap*)arg);
    }, newMap.release());

    // replace the name
    strncpy(_name, name, sizeof(_name));

    _arguments = std::move(arguments);

    // set new entry point and argv
    _entryPoint = (Thread::EntryPointFunction) elfLoader.getEntryPoint();
    _userArgv = userArgv;

    _reclaim_reent(&_reent);
    _REENT_INIT_PTR(&_reent);

    return true;
}

Process::~Process(void)
{
    try {
        _fileDescriptors.close_all();
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

uint64_t Process::cpu_time() const
{
    InterruptsMutex mutex(true);

    uint64_t time = 0;
    for (const auto& thread : _threads) {
        time += thread->cpuTime();
    }

    return time;
}

void Process::activateProcess(void)
{
    _REENT = &_reent;
    _memoryMap->activate();
}

void Process::setKernelProc(void) {
    _is_kernel_proc = true;
}

bool Process::is_kernel_proc(void) const {
    return _is_kernel_proc;
}

void Process::terminate(int exit_code) {
    InterruptsMutex mutex(true);
    _exitCode = exit_code;
    _state = Process::State::TERMINATED;
    _exitEvent.emit(exit_code);
}

bool Process::extendHeap(uintptr_t endAddr){
    auto* heapRegion = _memoryMap->get("heap");
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

VirtualMemoryRegion* Process::allocateStackRegion(std::string&& name, size_t size) {

    vm_prot_t prot = (vm_prot_t)(VM_PROT_READ | VM_PROT_WRITE);
    return _memoryMap->createMemoryRegionInRange(name.c_str(), 0x70000000, size, prot);
}

bool Process::createHeapRegion(ElfLoader& elfLoader, ProcessMemoryMap& memoryMap) {
    uintptr_t endAddr = elfLoader.getEndAddress();
    if (0 == endAddr) {
        kprintf("elfLoader: failed to obtain end address");
        return false;
    }

//    kprintf("Process: creating heap area from %08x to %08x\n", endAddr, endAddr + PAGESIZE);

    return nullptr != memoryMap.createMemoryRegion("heap", endAddr, endAddr + PAGESIZE,
                                                   (vm_prot_t)(VM_PROT_READ | VM_PROT_WRITE));
}

char** Process::createArgsRegion(ProcessMemoryMap& memoryMap, const std::vector<std::string>& arguments) {
    auto* argsRegion = memoryMap.createMemoryRegion("args",
                                                    0x70b00000, 0x70b01000,
                                                    (vm_prot_t) (VM_PROT_READ | VM_PROT_WRITE));
    if (!argsRegion) {
        return nullptr;
    }

    auto argv_elements = arguments.size() + 1; // for null ptr at the end
    auto argv_table_size = (sizeof(const char*) * argv_elements);
    int argv_size = argv_table_size;
    for (const auto& arg : arguments) {
        argv_size += arg.size() + 1; // plus null
    }

    char* user_argv = (char*) argsRegion->startAddress();
    memoryMap.runInScope([this, user_argv, argv_table_size, &arguments]() {
        char** argv_table = (char**) (user_argv);
        char* argv_data_pos = (user_argv + argv_table_size);
        size_t index;

        for (index = 0; index < arguments.size(); index++) {
            const auto& arg = arguments[index];
            argv_table[index] = argv_data_pos;

            strcpy(argv_data_pos, arg.c_str());
            argv_data_pos += arg.size();
            argv_data_pos += 1; // for the null
        }

        argv_table[index] = nullptr;
    });

    return (char**)user_argv;
}

bool Process::has_exited(void) const {
    return _state == State::TERMINATED;
}

int Process::wait_for_exit(void) {
    return _exitEvent.get();
}

Thread* Process::createThread(const char* name,
                              Thread::EntryPointFunction entryPoint, void* argument,
                              size_t stackSize,
                              bool schedule)
{

    auto thread = std::make_unique<Thread>(*this, name, entryPoint, argument, stackSize);
    if (!thread) {
        return nullptr;
    }

    kprintf("[%s] spawned a new thread with tid %d named %s\n", this->name(), thread->tid(), thread->name());

    return this->addThread(std::move(thread), schedule);
}

Thread* Process::cloneThread(const Thread& thread, Process& father, const struct user_regs* regs, bool schedule) {
    auto cloned_thread = std::make_unique<Thread>(thread, father, regs);
    if (!cloned_thread) {
        return nullptr;
    }

    return this->addThread(std::move(cloned_thread), schedule);
}

Thread* Process::addThread(std::unique_ptr<Thread> thread, bool schedule) {
    auto* thread_ptr = thread.get();

    assert(NULL != thread_ptr);

    {
        InterruptsMutex mutex(true);
        _threads.push_back(std::move(thread));
    }

    if (schedule && !Scheduler::instance().addThread(thread_ptr)) {
        panic("failed to add thread to scheduler");
    }

    return thread_ptr;
}

bool Process::changeWorkingDir(std::string&& path) {
    auto node = VirtualFileSystem::instance().lookup(Path(path));
    if (!node) {
        return false;
    }

    _cwdPath = Path(path);
    _cwdNode = node;
    return true;
}

const Path& Process::currentWorkingPath(void) {
    return _cwdPath;
}

SharedVFSNode Process::currentWorkingNode(void) {
    if (!_cwdNode) {
        this->changeWorkingDir(_cwdPath.string());
    }

    return _cwdNode;
}

bool Process::discardAllThreads(void) {
    auto& sched = Scheduler::instance();

    InterruptsMutex mutex(true);
    if (!sched.kill(this, false)) {
        return false;
    }

    for (const auto& thread : _threads) {
        sched.removeThread(thread.get());
    }

    // now we actually free the threads on a deferred workqueue job
    // to prevent us from pm_free() our own stack while running from it..
    auto oldThreads = std::make_unique<std::vector<std::unique_ptr<Thread>>>();
    assert(NULL != oldThreads);
    std::swap(_threads, *oldThreads);

    workqueue_put([](void* argument) -> void {
        delete((std::vector<std::unique_ptr<Thread>>*)argument);
    }, oldThreads.release());

    return true;
}
