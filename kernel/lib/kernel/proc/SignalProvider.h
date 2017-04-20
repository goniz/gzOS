#ifndef GZOS_SIGNALPROVIDER_H
#define GZOS_SIGNALPROVIDER_H

#include <sched.h>
#include <cstdint>

#ifdef __cplusplus

class ProcessProvider;
class Scheduler;
class SignalProvider
{
public:
    SignalProvider(ProcessProvider& processProvider, Scheduler& scheduler);

    bool signalPid(pid_t pid, int signal_nr, uintptr_t value);

    static SignalProvider& instance(void);

private:
    bool signalProcessPid(pid_t pid, int signal_nr, uintptr_t value);
    bool signalThreadTid(pid_t tid, int signal_nr, uintptr_t value);

    ProcessProvider& _processProvider;
    Scheduler& _scheduler;
};

#endif //cplusplus
#endif //GZOS_SIGNALPROVIDER_H
