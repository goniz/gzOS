#include <malloc.h>
#include <platform/sbrk.h>
#include <lib/mm/physmem.h>
#include <platform/cpu.h>
#include <platform/kprintf.h>
#include "malta.h"
#include "mips.h"

extern int _memsize;
extern char _stext;

void pm_boot() {
    pm_add_segment(MALTA_PHYS_SDRAM_BASE,
                   (pm_addr_t) (MALTA_PHYS_SDRAM_BASE + _memsize),
                   MIPS_KSEG0_START);

    const pm_addr_t start = platform_virt_to_phy((uintptr_t) &_stext);
    const pm_addr_t end = platform_virt_to_phy((uintptr_t) kernel_sbrk_shutdown());
    pm_reserve(start, end);
}
