#ifndef GZOS_SCHEDULINGPOLICYDATA_H
#define GZOS_SCHEDULINGPOLICYDATA_H

#ifdef __cplusplus

#define DefaultQuantum              (100)

struct SchedulingPolicyData
{

};

struct QuantumSchedulingPolicyData
{
    int quantum = DefaultQuantum;
    int resetQuantum = DefaultQuantum;
    bool responsive = false;
};

#endif //cplusplus
#endif //GZOS_SCHEDULINGPOLICYDATA_H
