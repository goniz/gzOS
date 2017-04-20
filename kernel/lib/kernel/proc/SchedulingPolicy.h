#ifndef GZOS_SCHEDULING_POLICY_H
#define GZOS_SCHEDULING_POLICY_H

#ifdef __cplusplus

class Thread;
class SchedulingPolicy
{
public:
    SchedulingPolicy(void) = default;
    virtual ~SchedulingPolicy(void) = default;

    virtual bool add(Thread* thread) = 0;
    virtual bool remove(Thread* thread) = 0;

    virtual Thread* choose(void) = 0;
    virtual Thread* evaluate_and_choose(Thread* thread) = 0;
    virtual bool can_choose(void) const = 0;
    virtual void suspend(Thread* thread) = 0;
    virtual void resume(Thread* thread) = 0;
};

#endif //cplusplus
#endif //GZOS_SCHEDULING_POLICY_H
