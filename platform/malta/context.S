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
        sw      $t0, (0  * 4)($sp)
        sw      $t1, (1  * 4)($sp)
        sw      $t2, (2  * 4)($sp)
        sw      $t3, (3  * 4)($sp)
        sw      $t4, (4  * 4)($sp)
        sw      $t5, (5  * 4)($sp)
        sw      $t6, (6  * 4)($sp)
        sw      $t7, (7  * 4)($sp)
        sw      $t8, (8  * 4)($sp)
        sw      $t9, (9  * 4)($sp)
        sw      $a0, (10 * 4)($sp)
        sw      $a1, (11 * 4)($sp)
        sw      $a2, (12 * 4)($sp)
        sw      $a3, (13 * 4)($sp)
        sw      $s0, (14 * 4)($sp)
        sw      $s1, (15 * 4)($sp)
        sw      $s2, (16 * 4)($sp)
        sw      $s3, (17 * 4)($sp)
        sw      $s4, (18 * 4)($sp)
        sw      $s5, (19 * 4)($sp)
        sw      $s6, (20 * 4)($sp)
        sw      $s7, (21 * 4)($sp)
        sw      $v0, (22 * 4)($sp)
        sw      $v1, (23 * 4)($sp)
        sw      $ra, (24 * 4)($sp)
        sw      $at, (25 * 4)($sp)
        sw      $fp, (26 * 4)($sp)
        sw      $gp, (27 * 4)($sp)

        mflo    $k0
        sw      $k0, (28 * 4)($sp)
        mfhi    $k0
        sw      $k0, (29 * 4)($sp)

    .ifeq \pc
        mfc0    $k0, C0_EPC
    .else
        move    $k0, $ra
    .endif
        sw      $k0, (30 * 4)($sp)

        move    \reg, $sp
.endm

.macro RESTORE_CTX pc=SAVE_CTX_PC_VIA_EPC
        /* Restore registers. */
        lw      $k0, (29 * 4)($sp)
        mthi    $k0

        lw      $k0, (28 * 4)($sp)
        mtlo    $k0

        lw      $gp, (27 * 4)($sp)
        lw      $fp, (26 * 4)($sp)
        lw      $at, (25 * 4)($sp)
        lw      $ra, (24 * 4)($sp)
        lw      $v1, (23 * 4)($sp)
        lw      $v0, (22 * 4)($sp)
        lw      $s7, (21 * 4)($sp)
        lw      $s6, (20 * 4)($sp)
        lw      $s5, (19 * 4)($sp)
        lw      $s4, (18 * 4)($sp)
        lw      $s3, (17 * 4)($sp)
        lw      $s2, (16 * 4)($sp)
        lw      $s1, (15 * 4)($sp)
        lw      $s0, (14 * 4)($sp)
        lw      $a3, (13 * 4)($sp)
        lw      $a2, (12 * 4)($sp)
        lw      $a1, (11 * 4)($sp)
        lw      $a0, (10 * 4)($sp)
        lw      $t9, (9  * 4)($sp)
        lw      $t8, (8  * 4)($sp)
        lw      $t7, (7  * 4)($sp)
        lw      $t6, (6  * 4)($sp)
        lw      $t5, (5  * 4)($sp)
        lw      $t4, (4  * 4)($sp)
        lw      $t3, (3  * 4)($sp)
        lw      $t2, (2  * 4)($sp)
        lw      $t1, (1  * 4)($sp)
        lw      $t0, (0  * 4)($sp)

        lw      $k0, (30 * 4)($sp)
    .ifeq \pc
        mtc0    $k0, C0_EPC
    .else
        move    $ra, $k0
    .endif

        /* Free stack frame */
        addu    $sp, $sp, REG_STACK_STORE_SIZE
.endm