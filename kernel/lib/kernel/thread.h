#ifndef GZOS_THREAD_H
#define GZOS_THREAD_H

#include <sched.h>

#ifdef __cplusplus
#include <cstdint>
#include <atomic>
#include <lib/mm/vm.h>

class Scheduler;
class Process;
class Thread
{
    friend class Scheduler;
    friend class Process;
    enum State { READY, RUNNING, SUSPENDED, TERMINATED };
    enum ContextType { UserSpace, KernelSpace };

    struct PreemptionContext {
        PreemptionContext(enum ContextType type, bool preemption_disallowed)
                : contextType(type), preemptionDisallowed(preemption_disallowed)
        {

        }

        enum ContextType contextType;
        bool preemptionDisallowed;
    };

public:
    using EntryPointFunction = int (*)(void*);

    Thread(Process& process,
           const char *name,
           EntryPointFunction entryPoint, void *argument,
           size_t stackSize, int initialQuantum);

    ~Thread(void);

    bool signal(int sig_nr);

    inline const char*  name(void) const        { return _name; }
    inline int          exit_code(void) const   { return _exitCode; }
    inline pid_t        tid(void)               { return _tid; }
    inline uint64_t     cpuTime(void)           { return _cpuTime; }
    inline Process&     proc(void)              { return _proc; }
    inline bool         isResponsive(void)      { return _responsive; }
    inline void         setResponsive(bool r)   { _responsive = r; }

private:
    __attribute__((noreturn))
    static void threadMainLoop(void* argument);

    struct user_regs* _context;
    PreemptionContext _preemptionContext;
    int _quantum;
    const int _resetQuantum;
    enum State _state;
    std::atomic_bool _responsive;
    char _name[64];
    pid_t _tid;
    int _exitCode;
    uint64_t _cpuTime;
    EntryPointFunction _entryPoint;
    void* _argument;
    Process& _proc;
    std::atomic_int _pending_signal_nr;
    void* _stackHead;
    size_t _stackSize;
};

#endif //cplusplus
#endif //GZOS_THREAD_H
