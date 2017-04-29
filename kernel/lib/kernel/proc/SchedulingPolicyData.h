#ifndef GZOS_SCHEDULINGPOLICYDATA_H
#define GZOS_SCHEDULINGPOLICYDATA_H

#ifdef __cplusplus

#define DefaultQuantum              (100)

struct SchedulingPolicyData
{
    bool yieldRequested = false;
};

struct QuantumSchedulingPolicyData : public SchedulingPolicyData
{
    int quantum = DefaultQuantum;
    int resetQuantum = DefaultQuantum;
};

#endif //cplusplus
#endif //GZOS_SCHEDULINGPOLICYDATA_H
