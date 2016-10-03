#include <asm.h>
#include <mips/m32c0.h>
#include "context.S"

.global platform_save_context
.global platform_restore_context
.global platform_context_switch

platform_save_context:
    mfc0 $t0, C0_STATUS
    # move $t1, $ra
    # move $ra, $a0
    SAVE_CTX $v0 SAVE_CTX_PC_VIA_RA

    # jr $t1
    jr $ra
    mtc0 $t0, C0_STATUS


platform_restore_context:
    mfc0 $t0, C0_STATUS
    nop

    mtc0 $v0, C0_USERLOCAL

    move $sp, $a0
    RESTORE_CTX SAVE_CTX_PC_VIA_RA
    move $v0, $zero

    mfc0 $v0, C0_USERLOCAL

    jr $ra
    mtc0 $t0, C0_STATUS

platform_context_switch:
    di $t0
    mtc0 $t0, C0_USERLOCAL

    SAVE_CTX $v0 SAVE_CTX_PC_VIA_RA

    sw  $v0, 0($a1)
    move $sp, $a0

    RESTORE_CTX SAVE_CTX_PC_VIA_RA

    mfc0 $k0, C0_USERLOCAL
    nop

    ei $k0
    jr $ra
