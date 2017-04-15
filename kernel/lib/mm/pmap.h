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

#ifndef _PMAP_H_
#define _PMAP_H_

#include <stdint.h>
#include <lib/primitives/sys/queue.h>
#include <platform/interrupts.h>
#include <stdbool.h>
#include "vm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t asid_t;
typedef uint32_t pte_t;
typedef uint32_t pde_t;

typedef struct pmap {
  pte_t *pte;          /* page table */
  pte_t *pde;          /* directory page table */
  vm_page_t *pde_page; /* pointer to a page with directory page table */
  TAILQ_HEAD(, vm_page) pte_pages; /* pages we allocate in page table */
  vm_addr_t start, end;
  asid_t asid;
} pmap_t;

void pmap_init();
pmap_t *pmap_new();
void pmap_reset(pmap_t *pmap);
void pmap_delete(pmap_t *pmap);

bool pmap_is_mapped(pmap_t *pmap, vm_addr_t vaddr);
bool pmap_is_range_mapped(pmap_t *pmap, vm_addr_t start, vm_addr_t end);

void pmap_map(pmap_t *pmap, vm_addr_t start, vm_addr_t end, pm_addr_t paddr,
              vm_prot_t prot);
void pmap_protect(pmap_t *pmap, vm_addr_t start, vm_addr_t end,
                  vm_prot_t prot);
void pmap_unmap(pmap_t *pmap, vm_addr_t start, vm_addr_t end);
int pmap_probe(pmap_t *pmap, vm_addr_t start, vm_addr_t end, vm_prot_t prot);

void pmap_activate(pmap_t *pmap);
pmap_t *get_kernel_pmap();
pmap_t *get_user_pmap();

struct user_regs *tlb_exception_handler(struct user_regs *regs);

#ifdef __cplusplus
}
#endif //extern "C"
#endif /* _PMAP_H_ */
