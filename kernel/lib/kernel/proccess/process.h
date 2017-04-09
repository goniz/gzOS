#ifndef GZOS_PROCESS_H
#define GZOS_PROCESS_H

#ifdef __cplusplus

#include <stdint.h>
#include <memory>
#include <functional>
#include <vector>
#include <sys/types.h>
#include <atomic>
#include "lib/kernel/vfs/FileDescriptorCollection.h"
#include "lib/kernel/proccess/ProcessMemoryMap.h"
#include "lib/kernel/elf/ElfLoader.h"
#include "lib/kernel/sched/thread.h"
#include <reent.h>
#include <lib/mm/vm_map.h>
#include <cassert>
#include <lib/primitives/EventStream.h>

/*
 * Process virtual address space:
 *
 *   stack  0x70000000 - 0x70a00000 (10MB)  User virtual memory, TLB mapped (useg)
 *   args   0x70b00000 - 0x70b01000 (4kb) User virtual memory, TLB mapped (useg)
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
            std::vector<std::string>&& arguments);

    ~Process(void);

    bool signal(int sig_nr);

    inline pid_t pid(void) const {
        assert(-1 != _pid);
        return _pid;
    }

    void terminate(int exit_code);

    bool extendHeap(uintptr_t endAddr);

    const char* name(void) const;
    const std::vector<std::string>& arguments(void) const;
    int exit_code(void) const;
    int state(void) const;
    uint64_t cpu_time(void) const;
    bool is_kernel_proc(void) const;
    bool traceme(bool state);
    bool traceme(void);
    bool has_exited(void) const;
    int wait_for_exit(void);

    FileDescriptorCollection& fileDescriptorCollection(void) {
        return _fileDescriptors;
    }

    std::vector<std::unique_ptr<Thread>>& threads(void) {
        return _threads;
    }

private:
    static void switchProcess(Process& newProc);

    Process(const char* name, std::vector<std::string>&& arguments, bool initializeFds = true);
    void createStackRegion();
    void createArgsRegion();

    char _name[64];
    const pid_t _pid;
    enum State _state;
    int _exitCode;
    EventStream<int> _exitEvent;
    Thread::EntryPointFunction _entryPoint;
    std::vector<std::string> _arguments;
    std::atomic<int> _pending_signal_nr;
    ProcessMemoryMap _memoryMap;
    FileDescriptorCollection _fileDescriptors;
    std::vector<std::unique_ptr<Thread>>   _threads;
    struct _reent _reent;
    bool _traceme;
    void* _userArgv;
};

#endif
#endif //GZOS_PROCESS_H
