#include <proc/Scheduler.h>
#include "lib/kernel/proc/proc.h"
#include <platform/panic.h>
#include <algorithm>
#include "ProcessProvider.h"
#include "proc.h"

ProcessProvider& ProcessProvider::instance(void) {
    auto* instance = process_provider_get();

    if (!instance) {
        panic("ProcessProvider instance is NULL. yayks..");
    }

    return *instance;
}

ProcessProvider::ProcessProvider(void)
    : _processList(),
      _systemProcess(nullptr)
{
    _processList.reserve(InitialProcSize);

    _systemProcess = this->createKernelProcess("kernel", std::vector<std::string>());
}

Process& ProcessProvider::SystemProcess(void) {
    return *_systemProcess;
}

Process* ProcessProvider::createKernelProcess(const char* name,
                                              std::vector<std::string>&& arguments,
                                              bool initializeFds)
{
    auto process = std::make_unique<Process>(name, std::move(arguments), initializeFds);
    if (!process) {
        return nullptr;
    }

    process->setKernelProc();

    kprintf("spawned a new kernel proc with pid %d named %s\n", process->pid(), process->name());

    auto processPtr = process.get();
    {
        InterruptsMutex mutex(true);
        _processList.push_back(std::move(process));
    }

    return processPtr;
}

Process* ProcessProvider::createProcess(const char* name,
                                        const void* buffer, size_t size,
                                        std::vector<std::string>&& arguments,
                                        size_t stackSize)
{
    ElfLoader loader(buffer, size);
    if (!loader.sanityCheck()) {
        return nullptr;
    }

    auto process = std::make_unique<Process>(name, loader, std::move(arguments));
    if (!process) {
        return nullptr;
    }

    kprintf("spawned a new proc with pid %d named %s\n", process->pid(), process->name());

    char thrd_name[128];
    sprintf(thrd_name, "%s_main", name);

    InterruptsMutex mutex(true);
    auto mainThread = process->createThread(thrd_name, process->entryPoint(), process->userArgv(), stackSize, true);
    if (!mainThread) {
        return nullptr;
    }

    auto processPtr = process.get();
    _processList.push_back(std::move(process));

    return processPtr;
}

bool ProcessProvider::execProcess(Process& proc,
                                  const char* name,
                                  const void* buffer, size_t size,
                                  std::vector<std::string>&& arguments,
                                  size_t stackSize)
{
    ElfLoader loader(buffer, size);
    if (!loader.sanityCheck()) {
        return false;
    }

    InterruptsMutex mutex(true);
    if (!proc.Exec(name, loader, std::move(arguments))) {
        kprintf("exec failed\n");
        return false;
    }

    kprintf("proc with pid %d renamed to %s\n", proc.pid(), proc.name());

    char thrd_name[128];
    sprintf(thrd_name, "%s_main", name);

    auto mainThread = proc.createThread(thrd_name, proc.entryPoint(), proc.userArgv(), stackSize, true);
    if (!mainThread) {
        return false;
    }

    return true;
}

Process* ProcessProvider::forkProcess(Process& proc, Thread& thread) {
    InterruptsMutex mutex(true);
    auto child = std::make_unique<Process>(proc);
    if (!child) {
        return nullptr;
    }

    Thread* newThread = child->cloneThread(thread, *child, true);
    if (!newThread) {
        return nullptr;
    }

    kprintf("proc %s (%d) forked to %d\n", proc.name(), proc.pid(), child->pid());


    auto childPtr = child.get();
    _processList.push_back(std::move(child));

    return childPtr;
}

Thread* ProcessProvider::getThreadByTid(pid_t tid) const {

    assert(PID_CURRENT != tid);

    InterruptsMutex mutex(true);

    for (const auto& proc : _processList) {
        for (const auto& thread : proc->threads()) {
            if (thread->tid() == tid) {
                return thread.get();
            }
        }
    }

    return nullptr;
}

Process* ProcessProvider::getProcessByPid(pid_t pid) const {

    assert(PID_CURRENT != pid);

    InterruptsMutex mutex(true);

    for (const auto& proc : _processList) {
        if (proc->pid() == pid) {
            return proc.get();
        }
    }

    return nullptr;
}

void ProcessProvider::destroyProcess(Process* proc) {
    InterruptsMutex mutex(true);

    for (const auto& thread : proc->threads()) {
        Scheduler::instance().removeThread(thread.get());
    }

    auto remove_iter = std::remove_if(_processList.begin(), _processList.end(), [proc](const auto& item) -> bool {
        return item.get() == proc;
    });

    _processList.erase(remove_iter, _processList.end());
}

Thread* ProcessProvider::createKernelThread(const char* name,
                                            Thread::EntryPointFunction entryPoint, void* argument,
                                            size_t stackSize,
                                            bool schedule)
{
    return _systemProcess->createThread(name, entryPoint, argument, stackSize, schedule);
}

void ProcessProvider::dumpProcesses(void) {
    InterruptsMutex mutex(true);

    kprintf("total %d procs:\n", _processList.size());

    for (const auto& proc : _processList) {

        kprintf("* proc %s (%d) with %d threads (%d%% cpu):\n",
                proc->name(),
                proc->pid(),
                proc->threads().size(),
                this->cpuPercentageOf(proc.get()));

        for (const auto& thread : proc->threads()) {
            kprintf("\t- %d thread %s cpu_time %d\n", thread->tid(), thread->name(), (uint32_t)thread->cpuTime());
        }
    }
}

int ProcessProvider::cpuPercentageOf(Process* proc) const {
    if (!proc) {
        return -1;
    }

    InterruptsMutex mutex(true);

    double total_time = 0;
    for (const auto& item : _processList) {
        total_time += item->cpu_time();
    }

    return ((unsigned int)proc->cpu_time() / total_time) * 100.0;
}
