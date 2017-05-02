#ifndef GZOS_PROC_H
#define GZOS_PROC_H

#ifdef __cplusplus

#include "lib/kernel/proc/Scheduler.h"
#include "lib/kernel/proc/ProcessProvider.h"
#include "lib/kernel/proc/SystemTimer.h"
#include "lib/kernel/proc/SignalProvider.h"

extern "C" ProcessProvider* process_provider_get(void);
extern "C" Scheduler* scheduler_get(void);
extern "C" SystemTimer* system_timer_get(void);
extern "C" SignalProvider* signal_provider_get(void);

#else

void* process_provider_get(void);
void* scheduler_get(void);
void* system_timer_get(void);
void* signal_provider_get(void);

#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TIMER_THATS_ENOUGH = 0,
    TIMER_KEEP_GOING = 1
} timeout_callback_ret;

typedef timeout_callback_ret (*timeout_callback_t)(void* scheduler, void* arg);
uint32_t scheduler_set_timeout(int timeout_ms, timeout_callback_t callback, void* arg);
void scheduler_unset_timeout(uint32_t timeoutId);

pid_t scheduler_current_pid(void);
pid_t scheduler_current_tid(void);

#ifdef __cplusplus
};
#endif

#endif //GZOS_PROC_H
