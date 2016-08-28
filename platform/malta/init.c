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

extern void system_init(int argc, const char **argv, const char **envp);
extern void malloc_init(void* start, size_t size);

void platform_init(int argc, const char **argv, const char **envp)
{
    uart_init();
    uart_puts("in platform_init!!\n");

    platform_read_cpu_config();
    platform_dump_additional_cpu_info();

    pm_init();
    tlb_init();
    vm_object_init();
    vm_map_init();

    vm_page_t* malloc_page = pm_alloc(PAGESIZE);
    if (NULL == malloc_page) {
        panic("Failed to alloc kmalloc pages");
    }

    malloc_init((void*)malloc_page->vaddr, (size_t) malloc_page->size * PAGESIZE);

	interrupts_init();
	clock_init();

    platform_pci_init();
	system_init(argc, argv, envp);
}
