#include "uart_cbus.h"
#include <platform/interrupts.h>
#include <platform/cpu.h>
#include <platform/panic.h>
#include "clock.h"
#include "tlb.h"
#include "malta.h"
#include "mips.h"
#include <platform/malta/pci.h>
#include <lib/mm/physmem.h>
#include <lib/mm/vm.h>
#include <lib/mm/vm_map.h>
#include <lib/mm/vm_object.h>
#include <lib/kernel/cmdline.h>
#include <lib/kernel/initrd/load_initrd.h>
#include <lib/primitives/align.h>

// Defined by the linker
extern char _stext;
extern char _end;
extern int _memsize;

extern void system_init(int argc, const char **argv, const char **envp);
extern void malloc_init(void* start, size_t size);

void platform_init(int argc, const char **argv, const char **envp)
{
    uart_init();

    platform_read_cpu_config();
    platform_dump_additional_cpu_info();

    cmdline_parse_arguments(argc, argv);

    pm_init();


    uintptr_t rd_start = (uintptr_t)initrd_get_address();
    size_t rd_size = align(initrd_get_size(), PAGESIZE);

    pm_seg_t* seg = (pm_seg_t*)(&_end + rd_size);
    size_t seg_size = align(pm_seg_space_needed(_memsize), PAGESIZE);

    /* create Malta physical memory segment */
    pm_seg_init(seg, MALTA_PHYS_SDRAM_BASE, MALTA_PHYS_SDRAM_BASE + _memsize, MIPS_KSEG0_START);

    /* reserve kernel image space */
    uart_puts("[malta] reserving kernel image memory\n");
    pm_seg_reserve(seg, MIPS_KSEG0_TO_PHYS((intptr_t)&_stext), MIPS_KSEG0_TO_PHYS((intptr_t)&_end));

    /* reserve ramdisk space */
    if (rd_start > 0) {
        uart_puts("[malta] reserving initrd image memory\n");
        pm_seg_reserve(seg, MIPS_KSEG0_TO_PHYS(rd_start), MIPS_KSEG0_TO_PHYS(rd_start + rd_size));
    }

    /* reserve segment description space */
    uart_puts("[malta] reserving segment description memory\n");
    pm_seg_reserve(seg, MIPS_KSEG0_TO_PHYS((intptr_t)seg), MIPS_KSEG0_TO_PHYS((intptr_t)seg + seg_size));

    pm_add_segment(seg);

    tlb_init();
    vm_object_init();
    vm_map_init();

    vm_page_t* malloc_page = pm_alloc(PAGESIZE);
    if (NULL == malloc_page) {
        panic("Failed to alloc kmalloc pages");
    }

    malloc_init((void*)malloc_page->vaddr, PG_SIZE(malloc_page));

	interrupts_init();
	clock_init();

    platform_pci_init();
	system_init(argc, argv, envp);
}
