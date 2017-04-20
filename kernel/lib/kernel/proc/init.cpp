#include <memory>
#include <platform/drivers.h>
#include "lib/kernel/proc/ProcessProvider.h"
#include "lib/kernel/proc/proc.h"
#include "lib/kernel/proc/Scheduler.h"
#include "lib/kernel/proc/PriorityScheduling.h"
#include "SystemTimer.h"
#include "SimpleRoundRobinSchedulingPolicy.h"
#include <lib/kernel/proc/active_scheduling_policy.h>

ProcessProvider* gProcessProviderInstance(nullptr);
Scheduler* gSchedulerInstance(nullptr);
SystemTimer* gSystemTimerInstace(nullptr);
SignalProvider* gSignalProviderInstance(nullptr);

static int proc_init(void)
{
    gProcessProviderInstance = new ProcessProvider();

    std::unique_ptr<SchedulingPolicy> policy = std::make_unique<ActiveSchedulingPolicyType>();
    gSchedulerInstance = new Scheduler(*gProcessProviderInstance, std::move(policy));
    gSchedulerInstance->initialize();
//    gSchedulerInstance->setDebugMode();

    gSystemTimerInstace = new SystemTimer(*gProcessProviderInstance, *gSchedulerInstance);
    gSignalProviderInstance = new SignalProvider(*gProcessProviderInstance, *gSchedulerInstance);

    return 0;
}

ProcessProvider* process_provider_get(void) {
    return gProcessProviderInstance;
}

Scheduler* scheduler_get(void) {
    return gSchedulerInstance;
}

SystemTimer* system_timer_get(void) {
    return gSystemTimerInstace;
}

SignalProvider* signal_provider_get(void) {
    return gSignalProviderInstance;
}

DECLARE_DRIVER(proc, proc_init, STAGE_FIRST);