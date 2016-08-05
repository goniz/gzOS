#include <asm.h>
#include <mips/m32c0.h>

        .set	noreorder       // Don't allow the assembler to reorder instructions.

        .local tlb_refill
        .local cache_error
        .local general_exception
        .local irq_handler
        .local irq0
        .local irq1
        .local irq2
        .local irq3
        .local irq4
        .local irq5
        .local irq6
        .local irq7

        .section .ebase

        .org 0x0

tlb_refill:
        mfc0 $k0, C0_CONTEXT        # Load UPT address which contains PTE
        sra $k0,1                   # PT should be located in KSEG2
        lw  $k1, 0($k0)
        mtc0 $k1, C0_ENTRYLO0
        lw  $k1, 4($k0)
        mtc0 $k1, C0_ENTRYLO1
        ehb                         # mtc0, Hazard on tlbwi
        tlbwr
        eret

        .org 0x100
cache_error:
1:      b       1b
        nop

        .org 0x180
general_exception:
        mfc0    $k0, C0_CAUSE
        andi    $k0, $k0, CR_X_MASK
        srl     $k0, $k0, CR_X_SHIFT
        li      $k1, EXC_SYS
        beq     $k1, $k0, 2f
        nop

        la      $k1, general_exception_table
        sll     $k0, $k0, 2   /* Multiply exc_code by sizeof(void *) */  
        add     $k1, $k1, $k0 /* Add exc_code */  
        lw      $k1, 0($k1)   /* Load the address of a handler */
        beqz    $k1, 1f       /* If NULL then call kernel_oops */
        nop
        // jr      $k1           /* Jump to handler */ 
		b		irq_handler
        nop

1:      jal     kernel_oops
        nop

2:      la		$k1, syscall_exception_handler
		b		irq_handler
        nop

        .org 0x200
irq0:
        la      $k1, mips_hw_irq0
        b       irq_handler
        nop

        .org 0x220
irq1:
        la      $k1, mips_hw_irq1
        b       irq_handler
        nop

        .org 0x240
irq2:
        la      $k1, mips_hw_irq2
        b       irq_handler
        nop

        .org 0x260
irq3:
        la      $k1, mips_hw_irq3
        b       irq_handler
        nop

        .org 0x280
irq4:
        la      $k1, mips_hw_irq4
        b       irq_handler
        nop

        .org 0x2a0
irq5:
        la      $k1, mips_hw_irq5
        b       irq_handler
        nop

        .org 0x2c0
irq6:
        la      $k1, mips_hw_irq6
        b       irq_handler
        nop

        .org 0x2e0
irq7:
        la      $k1, mips_hw_irq7
        b       irq_handler
        nop

        .org 0x300

#define REG_STACK_STORE_SIZE (31 * 4)

        .set	noat            // Don't allow the assembler to use r1(at) for synthetic instr.

irq_handler:
        /* Please, no nested interrupts for now. */
        di

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

        mfc0    $k0, C0_EPC
        sw      $k0, (30 * 4)($sp)

        # setup gp as per linker _gp symbol
        la      $gp, _gp
        move    $a0, $sp

        /* Call the C routine. */
        jalr    $k1
        nop

        /*
         * Support for ctx switch.
         * Switch $sp with the return value of the handler if not null
         */
        movn    $sp, $v0, $v0

        /* Restore registers. */
        lw      $k0, (30 * 4)($sp)
        mtc0    $k0, C0_EPC

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

        /* Free stack frame */
        addu    $sp, $sp, REG_STACK_STORE_SIZE

        /* Re-enable interrupts. */
        ei

        eret
