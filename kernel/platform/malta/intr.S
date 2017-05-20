#include <asm.h>
#include <mips/m32c0.h>
#include <platform/process.h>
#include "context.S"

.set	noreorder       // Don't allow the assembler to reorder instructions.
.set    noat

.global irq_handler
.global exception_enter
.global _thread_cb_pointer

.section .data
_thread_cb_pointer:
    .long 0

.section .text

// NOTE: expects $k1 to contain the handler function pointer
exception_enter:
    // check if the KSU SR bit says we're in user space
    // if we do, continue with 'exception_from_user', else 'exception_from_kernel'
    mfc0    $k0, C0_STATUS
    andi    $k0, SR_KSU_MASK
    beqz    $k0, exception_from_kernel
    nop

exception_from_user:
    la      $k0, _thread_cb_pointer
    lw      $k0, THREAD_STACK($k0)
    beqz    $k0, kernel_oops
    nop

    SAVE_CTX $k0, NOT_SP

    # Turn off FPU, enter kernel mode,
    # drop exception level and disable interrupts.
    mfc0    $t0, C0_STATUS
    li      $t1, ~(SR_CU1|SR_KSU_MASK|SR_EXL|SR_IE)
    and     $t0, $t1
    mtc0    $t0, C0_STATUS

    # Load kernel global pointer.
    la      $gp, _gp

    # Load kernel stack pointer
    move $sp, $k0

    # setup c handler regs param
    move $a0, $k0

    /* Call the C routine. */
    jalr    $k1
    nop

    # Interrupts may be enabled here and that's ok.

    # Disable interrupts till ERET.
    di

    la      $k0, _thread_cb_pointer
    addiu   $k1, $v0, REG_STACK_STORE_SIZE
    sw      $k1, THREAD_STACK($k0)

    /*
     * Support for ctx switch.
     * Switch $k0 with the return value of the handler
     */
    move    $k0, $v0

    RESTORE_CTX $k0, NOT_SP

    sync
    eret


exception_from_kernel:
    SAVE_CTX $sp, YES_SP

    # Drop exception level and disable interrupts.
    mfc0    $t0, C0_STATUS
    li      $t1, ~(SR_EXL|SR_IE)
    and     $t0, $t1
    mtc0    $t0, C0_STATUS

    # Load kernel global pointer.
    la      $gp, _gp

    # setup c handler regs param
    move $a0, $sp

    /* Call the C routine. */
    jalr    $k1
    nop

    # Interrupts may be enabled here and that's ok.
    # Disable interrupts till ERET.
    di

    la      $k0, _thread_cb_pointer
    addiu   $k1, $v0, REG_STACK_STORE_SIZE
    sw      $k1, THREAD_STACK($k0)

    /*
     * Support for ctx switch.
     * Switch $k0 with the return value of the handler
     */
    move    $sp, $v0

    RESTORE_CTX $sp, YES_SP

    sync
    eret
