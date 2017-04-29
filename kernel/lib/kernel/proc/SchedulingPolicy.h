#ifndef GZOS_SCHEDULING_POLICY_H
#define GZOS_SCHEDULING_POLICY_H

#ifdef __cplusplus

#include <cassert>
#include <lib/kernel/proc/SchedulingPolicyData.h>
#include <lib/kernel/proc/Thread.h>
#include <platform/panic.h>

class SchedulingPolicy
{
public:
    SchedulingPolicy(void) = default;
    virtual ~SchedulingPolicy(void) = default;

    virtual bool add(Thread* thread) = 0;
    virtual bool remove(Thread* thread) = 0;

    virtual Thread* choose(void) = 0;
    virtual Thread* evaluate_and_choose(Thread* thread) = 0;
    virtual Thread* yield(Thread* thread) = 0;
    virtual void suspend(Thread* thread) = 0;
    virtual void resume(Thread* thread) = 0;
};

template<class TData>
class BaseSchedulingPolicy : public SchedulingPolicy
{
protected:
    TData& dataFromThread(Thread* thread) const {
        TData* data = static_cast<TData*>(thread->schedulerData());

        if (!data) {
            panic("thread %s found with nulled scheduler data!", thread->name());
        }

        return *data;
    }
};

#endif //cplusplus
#endif //GZOS_SCHEDULING_POLICY_H
