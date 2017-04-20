#ifndef GZOS_SYSTEMTIMER_H
#define GZOS_SYSTEMTIMER_H

#include <cstdint>
#include <vector>
#include <lib/kernel/IdAllocator.h>
#include "Process.h"
#include "ProcessProvider.h"

#ifdef __cplusplus

class SystemTimer
{
public:
    using TimeoutCallbackFunc = bool (*)(void* arg);

    SystemTimer(ProcessProvider& processProvider, Scheduler& scheduler);

    uint32_t addTimer(int timeout_ms, TimeoutCallbackFunc cb, void* arg);
    void removeTimer(uint32_t timerId);

    Process& TimerProcess(void);

    static SystemTimer& instance(void);

private:
    void executeTimers(void);
    static bool onTimerTick(void* argument, struct user_regs** regs);

    struct TimerControlBlock {
        uint32_t id;
        uint64_t timeout_ms;
        uint64_t target_ms;
        TimeoutCallbackFunc callbackFunc;
        void* arg;
    };

    Process*                        _timerProc;
    std::vector<TimerControlBlock>  _timers;
    IdAllocator                     _idAllocator;
};

#endif //cplusplus
#endif //GZOS_SYSTEMTIMER_H
