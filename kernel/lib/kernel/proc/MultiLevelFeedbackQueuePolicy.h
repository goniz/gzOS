#ifndef GZOS_MULTILEVELFEEDBACKQUEUEPOLICY_H
#define GZOS_MULTILEVELFEEDBACKQUEUEPOLICY_H

#ifdef __cplusplus

#include "SchedulingPolicy.h"
#include "SchedulingPolicyData.h"
#include <lib/primitives/basic_queue.h>
#include <array>

struct MultiLevelFeedbackQueuePolicyData : public QuantumSchedulingPolicyData
{
    int runLevel = -1;
};

class MultiLevelFeedbackQueuePolicy : public BaseSchedulingPolicy<MultiLevelFeedbackQueuePolicyData>
{
public:
    MultiLevelFeedbackQueuePolicy(void);
    virtual ~MultiLevelFeedbackQueuePolicy(void) = default;

    virtual bool add(Thread* thread) override;
    virtual bool remove(Thread* thread) override;

    virtual Thread* choose(void) override;
    virtual Thread* evaluate_and_choose(Thread* thread) override;
    virtual Thread* yield(Thread* thread) override;
    virtual void suspend(Thread* thread) override;
    virtual void resume(Thread* thread) override;

private:
    static constexpr int RunLevelDepth = 10;
    static constexpr int RunLevels = 5;

    struct ThreadFIFO {
        int index;
        int quantum;
        basic_queue<Thread*> queue;
    };

    std::array<ThreadFIFO, RunLevels> _runLevels;
};


#endif //cplusplus
#endif //GZOS_MULTILEVELFEEDBACKQUEUEPOLICY_H
