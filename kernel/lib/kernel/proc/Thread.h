#ifndef GZOS_THREAD_H
#define GZOS_THREAD_H

#ifdef __cplusplus
#include <cstdint>
#include <atomic>
#include <memory>
#include <lib/mm/vm.h>
#include <platform/process.h>
#include "lib/kernel/proc/SchedulingPolicyData.h"
#include "ProcessMemoryMap.h"

class Scheduler;
class Process;
class Thread
{
    friend class Scheduler;
    friend class Process;

    enum ContextType { UserSpace, KernelSpace };
    static constexpr int KernelStackSize = PAGESIZE * 4;

    struct PreemptionContext {
        PreemptionContext(enum ContextType type, bool preemption_disallowed)
                : contextType(type), preemptionDisallowed(preemption_disallowed)
        {

        }

        enum ContextType contextType;
        bool preemptionDisallowed;
    };

public:
    enum State { READY, RUNNING, SUSPENDED, TERMINATED };
    using EntryPointFunction = int (*)(void*);

    Thread(Process& process,
           const char *name,
           EntryPointFunction entryPoint, void *argument,
           size_t stackSize);

    Thread(const Thread& other, Process& father, const struct user_regs* regs);

    ~Thread(void);

    bool sleep(int ms);
    bool signal(int sig_nr);
    void yield(void);
    bool is_in_kstack_range(void* ptr);

    inline const char*  name(void) const        { return _name; }
    inline int          exit_code(void) const   { return _exitCode; }
    inline pid_t        tid(void)               { return _tid; }
    inline uint64_t     cpuTime(void)           { return _cpuTime; }
    inline Process&     proc(void)              { return _proc; }
    inline enum State&  state(void)             { return _state; }

    void setSchedulerData(std::unique_ptr<SchedulingPolicyData> data) {
        _schedulingPolicyData = std::move(data);
    }

    SchedulingPolicyData* schedulerData(void) {
        return _schedulingPolicyData.get();
    }

private:
    vm_page_t* _kernelStackPage;
    platform_thread_cb _platformThreadCb;
    PreemptionContext _preemptionContext;
    std::unique_ptr<SchedulingPolicyData> _schedulingPolicyData;
    enum State _state;
    char _name[64];
    pid_t _tid;
    int _exitCode;
    uint64_t _cpuTime;
    EntryPointFunction _entryPoint;
    void* _argument;
    Process& _proc;
    std::atomic_int _pending_signal_nr;
    VirtualMemoryRegion* _userStackRegion;
};

#endif //cplusplus
#endif //GZOS_THREAD_H
