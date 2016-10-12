//
// Created by gz on 6/11/16.
//

#ifndef GZOS_INTERRUPTS_H
#define GZOS_INTERRUPTS_H

#ifndef __ASSEMBLER__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct user_regs {
    uint32_t t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
    uint32_t a0, a1, a2, a3;
    uint32_t s0, s1, s2, s3, s4, s5, s6, s7;
    uint32_t v0, v1;
    uint32_t sp, fp;
    uint32_t hi, lo;
    uint32_t epc, ra;
    uint32_t status, cause, badvaddr;
    uint32_t at;
    uint32_t gp;
};

#ifdef __cplusplus
}
#endif //cplusplus

#else
#define REG_T0  (0 * 4)
#define REG_T1 	(1 * 4)
#define REG_T2 	(2 * 4)
#define REG_T3 	(3 * 4)
#define REG_T4 	(4 * 4)
#define REG_T5 	(5 * 4)
#define REG_T6 	(6 * 4)
#define REG_T7 	(7 * 4)
#define REG_T8 	(8 * 4)
#define REG_T9 	(9 * 4)

#define REG_A0 	(10 * 4)
#define REG_A1 	(11 * 4)
#define REG_A2 	(12 * 4)
#define REG_A3 	(13 * 4)

#define REG_S0 	(14 * 4)
#define REG_S1 	(15 * 4)
#define REG_S2 	(16 * 4)
#define REG_S3 	(17 * 4)
#define REG_S4 	(18 * 4)
#define REG_S5 	(19 * 4)
#define REG_S6 	(20 * 4)
#define REG_S7 	(21 * 4)

#define REG_V0 	(22 * 4)
#define REG_V1 	(23 * 4)

#define REG_SP 	(24 * 4)
#define REG_FP 	(25 * 4)

#define REG_HI 	(26 * 4)
#define REG_LO 	(27 * 4)

#define REG_EPC	(28 * 4)
#define REG_RA 	(29 * 4)

#define REG_STATUS	(30 * 4)
#define REG_CAUSE 	(31 * 4)
#define REG_BADVA 	(32 * 4)

#define REG_AT	(33 * 4)
#define REG_GP 	(34 * 4)
#endif // __ASSEMBLER__

#endif //GZOS_INTERRUPTS_H
