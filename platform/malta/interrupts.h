//
// Created by gz on 6/11/16.
//

#ifndef GZOS_INTERRUPTS_H
#define GZOS_INTERRUPTS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct user_regs {
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t t7;
    uint32_t t8;
    uint32_t t9;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t v0;
    uint32_t v1;
    uint32_t ra;
    uint32_t lo;
    uint32_t hi;
};

#ifdef __cplusplus
}
#endif
#endif //GZOS_INTERRUPTS_H
