#ifndef GZOS_PROCESS_H
#define GZOS_PROCESS_H

#ifdef __cplusplus

#include <stdint.h>
#include <memory>
#include <functional>
#include <vector>
#include <sys/types.h>
#include <atomic>
#include "FileDescriptorCollection.h"
#include "ProcessMemoryMap.h"
#include "ElfLoader.h"
#include "thread.h"
#include <reent.h>
#include <lib/mm/vm_map.h>
#include <cassert>

/*
 * Process virtual address space:
 *
 *   stack  0x70000000 - 0x70a00000 (10MB)  User virtual memory, TLB mapped (useg)
 */

class Scheduler;
class Thread;

class Process
{
    friend class Scheduler;
    friend class Thread;
    enum State { READY, RUNNING, SUSPENDED, TERMINATED };

public:
    Process(const char* name,
            ElfLoader& elfLoader,
            std::vector<const char*>&& arguments);

    ~Process(void);

    bool signal(int sig_nr);

    inline pid_t pid(void) const {
        assert(-1 != _pid);
        return _pid;
    }

    void terminate(int exit_code);

    bool extendHeap(uintptr_t endAddr);

    const char* name(void) const;
    int exit_code(void) const;
    int state(void) const;
    uint64_t cpu_time(void) const;
    bool is_kernel_proc(void) const;

    FileDescriptorCollection& fileDescriptorCollection(void) {
        return _fileDescriptors;
    }

    std::vector<std::unique_ptr<Thread>>& threads(void) {
        return _threads;
    }

private:
    Process(const char* name, std::vector<const char*>&& arguments);

    static void switchProcess(Process& newProc);

    char _name[64];
    const pid_t _pid;
    enum State _state;
    int _exitCode;
    Thread::EntryPointFunction _entryPoint;
    std::vector<const char*> _arguments;
    std::atomic<int> _pending_signal_nr;
    ProcessMemoryMap _memoryMap;
    FileDescriptorCollection _fileDescriptors;
    std::vector<std::unique_ptr<Thread>>   _threads;
    struct _reent _reent;
};

#endif
#endif //GZOS_PROCESS_H
