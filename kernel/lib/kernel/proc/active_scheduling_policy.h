#ifndef GZOS_ACTIVE_SCHEDULING_POLICY_H
#define GZOS_ACTIVE_SCHEDULING_POLICY_H
#ifdef __cplusplus

#include "lib/kernel/proc/SimpleRoundRobinSchedulingPolicy.h"
#include "lib/kernel/proc/MultiLevelFeedbackQueuePolicy.h"

using ActiveSchedulingPolicyType = MultiLevelFeedbackQueuePolicy;
//using ActiveSchedulingPolicyType = SimpleRoundRobinSchedulingPolicy;

#endif //extern "C"
#endif //GZOS_ACTIVE_SCHEDULING_POLICY_H
