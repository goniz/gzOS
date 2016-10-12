#include "interrupts.h"

#define REG_STACK_STORE_SIZE (31 * 4)
#define SAVE_CTX_PC_VIA_EPC  0
#define SAVE_CTX_PC_VIA_RA   1

.set	noat            // Don't allow the assembler to use r1(at) for synthetic instr.

.macro SAVE_CTX reg, pc=SAVE_CTX_PC_VIA_EPC
        /* Allocate space for register storage on the stack. */
        subu    $sp, $sp, REG_STACK_STORE_SIZE

        /*
         * Save registers on the stack.
         * Saving all t, a and v registers, as well as LO and HI.
         * k registers do not require saving.
         */
        sw      $t0, REG_T0($sp)
        sw      $t1, REG_T1($sp)
        sw      $t2, REG_T2($sp)
        sw      $t3, REG_T3($sp)
        sw      $t4, REG_T4($sp)
        sw      $t5, REG_T5($sp)
        sw      $t6, REG_T6($sp)
        sw      $t7, REG_T7($sp)
        sw      $t8, REG_T8($sp)
        sw      $t9, REG_T9($sp)
        sw      $a0, REG_A0($sp)
        sw      $a1, REG_A1($sp)
        sw      $a2, REG_A2($sp)
        sw      $a3, REG_A3($sp)
        sw      $s0, REG_S0($sp)
        sw      $s1, REG_S1($sp)
        sw      $s2, REG_S2($sp)
        sw      $s3, REG_S3($sp)
        sw      $s4, REG_S4($sp)
        sw      $s5, REG_S5($sp)
        sw      $s6, REG_S6($sp)
        sw      $s7, REG_S7($sp)
        sw      $v0, REG_V0($sp)
        sw      $v1, REG_V1($sp)
        sw      $ra, REG_RA($sp)
        sw      $at, REG_AT($sp)
        sw      $fp, REG_FP($sp)
        sw      $gp, REG_GP($sp)

		addiu	$t0, $sp, REG_STACK_STORE_SIZE
		sw		$t0, REG_SP($sp)

        mflo    $t0
        mfhi    $t1
        sw      $t0, REG_LO($sp)
        sw      $t1, REG_HI($sp)

        mfc0    $t0, C0_EPC
        mfc0    $t1, C0_STATUS
        mfc0    $t2, C0_CAUSE
        mfc0    $t3, C0_BADVADDR
        sw      $t0, REG_EPC($sp)
        sw      $t1, REG_STATUS($sp)
        sw      $t2, REG_CAUSE($sp)
        sw      $t3, REG_BADVA($sp)

        move    \reg, $sp
.endm

.macro RESTORE_CTX pc=SAVE_CTX_PC_VIA_EPC
        # Update status register held in user context (interrupt mask).
        lw      $t0, REG_STATUS($sp)
        li      $t1, ~SR_IMASK
        and     $t0, $t1
        mfc0    $t1, C0_STATUS
        andi    $t1, SR_IMASK
        or      $t0, $t1
        sw      $t0, REG_STATUS($sp)

        /* Restore registers. */
        lw      $t0, REG_HI($sp)
        lw      $t1, REG_LO($sp)
        mthi    $t0
        mtlo    $t1

        lw      $gp, REG_GP($sp)
        lw      $fp, REG_FP($sp)
        lw      $at, REG_AT($sp)
        lw      $ra, REG_RA($sp)
        lw      $v1, REG_V1($sp)
        lw      $v0, REG_V0($sp)
        lw      $s7, REG_S7($sp)
        lw      $s6, REG_S6($sp)
        lw      $s5, REG_S5($sp)
        lw      $s4, REG_S4($sp)
        lw      $s3, REG_S3($sp)
        lw      $s2, REG_S2($sp)
        lw      $s1, REG_S1($sp)
        lw      $s0, REG_S0($sp)
        lw      $a3, REG_A3($sp)
        lw      $a2, REG_A2($sp)
        lw      $a1, REG_A1($sp)
        lw      $a0, REG_A0($sp)
        lw      $t9, REG_T9($sp)
        lw      $t8, REG_T8($sp)
        lw      $t7, REG_T7($sp)
        lw      $t6, REG_T6($sp)
        lw      $t5, REG_T5($sp)
        lw      $t4, REG_T4($sp)
        lw      $t3, REG_T3($sp)
        lw      $t2, REG_T2($sp)
        lw      $t1, REG_T1($sp)
        lw      $t0, REG_T0($sp)

        lw      $k0, REG_EPC($sp)
    .ifeq \pc
        mtc0    $k0, C0_EPC
    .else
        move    $ra, $k0
    .endif

        lw      $k0, REG_STATUS($sp)
        mtc0    $k0, C0_STATUS

        /* Free stack frame */
        addu    $sp, $sp, REG_STACK_STORE_SIZE
.endm
