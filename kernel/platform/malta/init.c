#include "uart_cbus.h"
#include <platform/interrupts.h>
#include <platform/cpu.h>
#include <platform/panic.h>
#include "clock.h"
#include "tlb.h"
#include <platform/malta/pci.h>
#include <lib/mm/physmem.h>
#include <lib/mm/vm.h>
#include <lib/mm/vm_map.h>
#include <lib/mm/vm_object.h>
#include <lib/kernel/cmdline.h>
#include <lib/kernel/initrd/load_initrd.h>
#include <platform/sbrk.h>

extern void system_init(int argc, const char **argv, const char **envp);
extern void malloc_init(void* start, size_t size);

void platform_init(int argc, const char **argv, const char **envp)
{
    uart_init();

    platform_read_cpu_config();
    platform_dump_additional_cpu_info();

    cmdline_parse_arguments(argc, argv);

    size_t rd_size = initrd_get_size();
    if (rd_size > 0) {
        uart_puts("[initrd] reserving memory for initrd\n");
        kernel_sbrk(rd_size);
    }

    pm_init();
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
