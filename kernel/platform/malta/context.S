#include "interrupts.h"

#define REG_STACK_STORE_SIZE (35 * 4)
#define NOT_SP  0
#define YES_SP  1

.set	noreorder
.set    noat

.macro SAVE_CTX reg, is_sp=0
        /* Allocate space for register storage on the stack. */
        subu    \reg, \reg, REG_STACK_STORE_SIZE

        /*
         * Save registers on the stack.
         * Saving all t, a and v registers, as well as LO and HI.
         * k registers do not require saving.
         */
        sw      $t0, REG_T0(\reg)
        sw      $t1, REG_T1(\reg)
        sw      $t2, REG_T2(\reg)
        sw      $t3, REG_T3(\reg)
        sw      $t4, REG_T4(\reg)
        sw      $t5, REG_T5(\reg)
        sw      $t6, REG_T6(\reg)
        sw      $t7, REG_T7(\reg)
        sw      $t8, REG_T8(\reg)
        sw      $t9, REG_T9(\reg)
        sw      $a0, REG_A0(\reg)
        sw      $a1, REG_A1(\reg)
        sw      $a2, REG_A2(\reg)
        sw      $a3, REG_A3(\reg)
        sw      $s0, REG_S0(\reg)
        sw      $s1, REG_S1(\reg)
        sw      $s2, REG_S2(\reg)
        sw      $s3, REG_S3(\reg)
        sw      $s4, REG_S4(\reg)
        sw      $s5, REG_S5(\reg)
        sw      $s6, REG_S6(\reg)
        sw      $s7, REG_S7(\reg)
        sw      $v0, REG_V0(\reg)
        sw      $v1, REG_V1(\reg)
        sw      $ra, REG_RA(\reg)
        sw      $at, REG_AT(\reg)
        sw      $fp, REG_FP(\reg)
        sw      $gp, REG_GP(\reg)

// if reg is not $sp
.ifeq \is_sp
        sw      $sp, REG_SP(\reg)
.else
        addiu	$t0, $sp, REG_STACK_STORE_SIZE
		sw		$t0, REG_SP(\reg)
.endif

        mflo    $t0
        mfhi    $t1
        sw      $t0, REG_LO(\reg)
        sw      $t1, REG_HI(\reg)

        mfc0    $t0, C0_EPC
        mfc0    $t1, C0_STATUS
        mfc0    $t2, C0_CAUSE
        mfc0    $t3, C0_BADVADDR
        sw      $t0, REG_EPC(\reg)
        sw      $t1, REG_STATUS(\reg)
        sw      $t2, REG_CAUSE(\reg)
        sw      $t3, REG_BADVA(\reg)
.endm

// NOTE: uses $k1 in the restore process
.macro RESTORE_CTX reg, is_sp=0
        # Update status register held in user context:
        #   * set right interrupt mask.
        #   * raise exception level
        lw      $t0, REG_STATUS(\reg)
        li      $t1, ~SR_IMASK
        and     $t0, $t1
        mfc0    $t1, C0_STATUS
        andi    $t1, SR_IMASK
        or      $t0, $t1
        ori     $t0, SR_EXL
        sw      $t0, REG_STATUS(\reg)

        /* Restore registers. */
        lw      $t0, REG_HI(\reg)
        lw      $t1, REG_LO(\reg)
        mthi    $t0
        mtlo    $t1

        lw      $gp, REG_GP(\reg)
        lw      $fp, REG_FP(\reg)
        lw      $at, REG_AT(\reg)
        lw      $ra, REG_RA(\reg)
        lw      $v1, REG_V1(\reg)
        lw      $v0, REG_V0(\reg)
        lw      $s7, REG_S7(\reg)
        lw      $s6, REG_S6(\reg)
        lw      $s5, REG_S5(\reg)
        lw      $s4, REG_S4(\reg)
        lw      $s3, REG_S3(\reg)
        lw      $s2, REG_S2(\reg)
        lw      $s1, REG_S1(\reg)
        lw      $s0, REG_S0(\reg)
        lw      $a3, REG_A3(\reg)
        lw      $a2, REG_A2(\reg)
        lw      $a1, REG_A1(\reg)
        lw      $a0, REG_A0(\reg)
        lw      $t9, REG_T9(\reg)
        lw      $t8, REG_T8(\reg)
        lw      $t7, REG_T7(\reg)
        lw      $t6, REG_T6(\reg)
        lw      $t5, REG_T5(\reg)
        lw      $t4, REG_T4(\reg)
        lw      $t3, REG_T3(\reg)
        lw      $t2, REG_T2(\reg)
        lw      $t1, REG_T1(\reg)
        lw      $t0, REG_T0(\reg)

        lw      $k1, REG_EPC(\reg)
        mtc0    $k1, C0_EPC

        lw      $k1, REG_STATUS(\reg)
        mtc0    $k1, C0_STATUS

// if reg is not $sp
.ifeq \is_sp
        lw      $sp, REG_SP(\reg)
        /* Free stack frame */
        addu    \reg, \reg, REG_STACK_STORE_SIZE
.else
        lw      $sp, REG_SP(\reg)
.endif
.endm
