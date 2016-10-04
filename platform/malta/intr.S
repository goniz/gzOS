#include <asm.h>
#include <mips/m32c0.h>
#include "context.S"

        .set	noreorder       // Don't allow the assembler to reorder instructions.

		.global is_in_irq
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

		.section .data
is_in_irq:
		.word 0

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

        .set	noat            // Don't allow the assembler to use r1(at) for synthetic instr.

irq_handler:
        /* Please, no nested interrupts for now. */
        di

        SAVE_CTX $a0 SAVE_CTX_PC_VIA_EPC

		/* store non-zero value in `is_in_irq` */
		la		$k0, is_in_irq
		sw		$k0, 0($k0)

        /* Call the C routine. */
        jalr    $k1
        nop

		/* store zero value in `is_in_irq` */
		la		$k0, is_in_irq
		sw		$zero, 0($k0)

        /*
         * Support for ctx switch.
         * Switch $sp with the return value of the handler if not null
         */
        movn    $sp, $v0, $v0

        RESTORE_CTX SAVE_CTX_PC_VIA_EPC

        /* Re-enable interrupts. */
        ei

        eret
