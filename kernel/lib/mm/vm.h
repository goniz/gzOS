/* The MIT License (MIT)              
                    
    Copyright (c) 2015 Krystian Bacławski
                    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
        
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER   
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.           
              
    Source code was adapted from https://github.com/cahirwpz/mimiker
*/

#ifndef _VIRT_MEM_H_
#define _VIRT_MEM_H_

#include <lib/primitives/sys/queue.h>
#include <stdint.h>
#include <lib/primitives/sys/tree.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PAGESIZE 4096

#define PG_SIZE(pg) ((pg)->size * PAGESIZE)
#define PG_START(pg) ((pg)->paddr)
#define PG_END(pg) ((pg)->paddr + PG_SIZE(pg))
#define PG_VADDR_START(pg) ((pg)->vaddr)
#define PG_VADDR_END(pg) ((pg)->vaddr + PG_SIZE(pg))

#define PM_RESERVED   1  /* non releasable page */
#define PM_ALLOCATED  2  /* page has been allocated */
#define PM_MANAGED    4  /* a page is on a freeq */

#define VM_ACCESSED 1  /* page has been accessed since last check */
#define VM_MODIFIED 2  /* page has been modified since last check */

typedef uintptr_t vm_addr_t;
typedef uintptr_t pm_addr_t;
typedef intptr_t vm_paddr_t;
typedef intptr_t vm_offset_t;
typedef uintptr_t vm_size_t;

typedef enum {
    VM_PROT_NONE = 0,
    VM_PROT_READ = 1,
    VM_PROT_WRITE = 2,
    VM_PROT_EXEC = 4
} vm_prot_t;

typedef struct vm_page {
    union {
        TAILQ_ENTRY(vm_page) freeq; /* list of free pages for buddy system */
        struct {
            TAILQ_ENTRY(vm_page) list;
            RB_ENTRY(vm_page) tree;
        } obj;
        struct {
            TAILQ_ENTRY(vm_page) list;
        } pt;
    };
    vm_addr_t vm_offset;          /* offset to page in vm_object */
    vm_addr_t vaddr;              /* virtual address of page */
    pm_addr_t paddr;              /* physical address of page */
    vm_prot_t prot;               /* page access rights */
    uint8_t vm_flags;             /* flags used by virtual memory system */
    uint8_t pm_flags;             /* flags used by physical memory system */
    uint32_t size;                /* size of page in PAGESIZE units */
} vm_page_t;

TAILQ_HEAD(pg_list, vm_page);
typedef struct pg_list pg_list_t;

typedef struct vm_map vm_map_t;
typedef struct vm_map_entry vm_map_entry_t;
typedef struct vm_object vm_object_t;
typedef struct pager pager_t;

#ifdef __cplusplus
}
#endif
#endif /* _VIRT_MEM_H_ */
