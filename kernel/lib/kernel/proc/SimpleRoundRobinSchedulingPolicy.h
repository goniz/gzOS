#ifndef GZOS_SIMPLEROUNDROBINSCHEDULINGPOLICY_H
#define GZOS_SIMPLEROUNDROBINSCHEDULINGPOLICY_H

#ifdef __cplusplus

#include "lib/kernel/proc/SchedulingPolicy.h"
#include <lib/primitives/basic_queue.h>
#include "lib/kernel/proc/SchedulingPolicyData.h"

class Thread;
class SimpleRoundRobinSchedulingPolicy : public SchedulingPolicy
{
public:
    using PolicyDataType = QuantumSchedulingPolicyData;

    SimpleRoundRobinSchedulingPolicy(void);
    virtual ~SimpleRoundRobinSchedulingPolicy(void) = default;

    virtual bool add(Thread* thread) override;
    virtual bool remove(Thread* thread) override;

    virtual Thread* choose(void) override;
    virtual Thread* evaluate_and_choose(Thread* thread) override;
    virtual bool can_choose(void) const override;
    virtual void suspend(Thread* thread) override;
    virtual void resume(Thread* thread) override;

private:
    static constexpr int ReadyQueueCapacity = 50;

    basic_queue<Thread*> _readyQueue;
};


#endif //cplusplus
#endif //GZOS_SIMPLEROUNDROBINSCHEDULINGPOLICY_H
