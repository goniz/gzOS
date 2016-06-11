#include <asm.h>
#include <mips/m32c0.h>
#include <platform/malta/mips.h>

        .set noreorder

        .local clear_bss
        .local kernel_go

# Boot environment is described in MIPS® YAMON™ User's Manual, Chapter 5
#
# $a0 = argc
# $a1 = argv
# $a2 = envp
# $a3 = memsize
#
# $sp is initialized

LEAF(_start)
		# write $a3 (memsize param) to _memsize gloabl
        la      $t0, _memsize
        sw      $a3,($t0)
		# setup gp as per linker _gp symbol
        la      $gp, _gp
		# setup stack pointer to the end of ram 
		li		$t0, MIPS_KSEG0_START
		addu	$sp, $t0, $a3

clear_bss:
        la      $t0, __bss
        la      $t1, __ebss

1:      sw      $zero, ($t0)
        bne     $t0, $t1, 1b
        addiu   $t0, 4

kernel_go:
        # Status.ERL is set by the processor when a Reset, Soft Reset, NMI or
        # Cache Error exception are taken. When ERL is set:
        # - The processor is running in kernel mode
        # - Hardware and software interrupts are disabled
        # - The ERET instruction will use the return address held in ErrorEPC
        #   instead of EPC

        la      $ra, kernel_exit
        la      $t0, platform_init
        mtc0    $t0, C0_EPC    # Return from exception to platform_init
		ehb
        eret                     # exception => normal level (SR_ERL = 0)
		nop
END(_start)

LEAF(kernel_exit)
1:      j       1b
        nop
END(kernel_exit)

LEAF(_get_stack_pointer)
	jr $ra
	addu $v0, $zero, $sp
END(_get_stack_pointer)

        .globl _memsize
        .type  _memsize,@object
        .size  _memsize,4

        .data
_memsize:
        .long 0
