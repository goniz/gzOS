#ifndef GZOS_PROCESS_H
#define GZOS_PROCESS_H

#include <stdint.h>
#include <memory>
#include <functional>
#include <vector>
#include <sys/types.h>
#include <atomic>
#include "FileDescriptorCollection.h"

class ProcessScheduler;
class Process
{
    friend class ProcessScheduler;
    enum State { READY, RUNNING, SUSPENDED, YIELDING, TERMINATED };
    enum PreemptionType { Responsive, Preemptive };
    enum ContextType { UserSpace, KernelSpace };

public:
    using EntryPointFunction = int (*)(int, const char**);

    Process(const char* name,
            EntryPointFunction entryPoint, std::vector<const char*>&& arguments,
            size_t stackSize,
            enum PreemptionType procType, int initialQuantum);

    ~Process(void);

    bool signal(int sig_nr);

    inline pid_t pid(void) const {
        return _pid;
    }

    const char* name(void) const;
    int exit_code(void) const;
    int state(void) const;
    int type(void) const;
    uint64_t cpu_time(void) const;

    FileDescriptorCollection& fileDescriptorCollection(void) {
        return _fileDescriptors;
    }

private:
    __attribute__((noreturn))
    static void processMainLoop(void* argument);

    struct PreemptionContext {
        PreemptionContext(enum ContextType type, bool preemption_disallowed)
                : contextType(type), preemptionDisallowed(preemption_disallowed)
        {

        }

        enum ContextType contextType;
        bool preemptionDisallowed;
    };

    struct user_regs* _context;
    PreemptionContext _preemtionContext;
    int _quantum;
    const int _resetQuantum;
    enum State _state;
    char _name[64];
    const pid_t _pid;
    const enum PreemptionType _preemptionType;
    int _exitCode;
    uint64_t _cpuTime;
    EntryPointFunction _entryPoint;
    std::vector<const char*> _arguments;
	struct platform_process_ctx* _pctx;
    std::atomic<int> _pending_signal_nr;
    FileDescriptorCollection _fileDescriptors;
};

#endif //GZOS_PROCESS_H
