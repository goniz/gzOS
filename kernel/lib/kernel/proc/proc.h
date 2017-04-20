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
#endif //GZOS_PROC_H
