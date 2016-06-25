#ifndef _PHYS_MEM_H_
#define _PHYS_MEM_H_

#include <stdint.h>
#include "vm.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize physical memory manager. */
void pm_init();

/* 
 * This adds segment to be managed by the vm_phys subsystem. 
 * In this function system uses kernel_sbrk function to allocate some data to 
 * manage pages. After kernel_sbrk_shutdown this function shouldn't be used.   
 * Note that this system manages PHYSICAL PAGES. Therefore start and end,
 * should be physical addresses. vm_offset determines initial virt_addr
 * for every page allocated from system. All pages in this segment will have
 * their default vm_addresses in range (start + vm_offset, end + vm_offset).
 */
void pm_add_segment(pm_addr_t start, pm_addr_t end, vm_addr_t vm_offset);

/* Allocates contiguous big page that consists of n machine pages. */
vm_page_t *pm_alloc(size_t n);

vm_page_t* pm_alloc_bytes(size_t nb);

void pm_free(vm_page_t *page);

void pm_dump();

/* After using this function pages in range (start, end) are never going to be
 * allocated. Should be used at start to avoid allocating from text, data,
 * ebss, or any possibly unwanted places. */
void pm_reserve(pm_addr_t start, pm_addr_t end);

#ifdef __cplusplus
}
#endif
#endif /* _PHYS_MEM_H */

