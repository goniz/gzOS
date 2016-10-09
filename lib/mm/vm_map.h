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

#ifndef _VM_MAP_H_
#define _VM_MAP_H_

#include "vm.h"
#include "pmap.h"

#ifdef __cplusplus
extern "C" {
#endif

/* There has to be distinction between kernel and user vm_map,
 * That's because in current implementation page table is always located in
 * KSEG2, while user vm_map address range contains no KSEG2 */

typedef enum { KERNEL_VM_MAP = 1, USER_VM_MAP = 2 } vm_map_type_t;

typedef struct vm_map_entry vm_map_entry_t;

struct vm_map_entry {
    TAILQ_ENTRY(vm_map_entry) map_list;
    SPLAY_ENTRY(vm_map_entry) map_tree;
    vm_object_t *object;

    uint32_t prot;
    vm_addr_t start;
    vm_addr_t end;
};

typedef struct vm_map {
    TAILQ_HEAD(, vm_map_entry) list;
    SPLAY_HEAD(vm_map_tree, vm_map_entry) tree;
    size_t nentries;
    pmap_t pmap;
} vm_map_t;

/* TODO we will need some functions to allocate address ranges,
 * since vm_map contains all information about address ranges, it's best idea
 * to embed allocation into this subsystem instead of building new one
 * on top of it. Function of similar interface would be basic:
 *
 * vm_map_entry_t* vm_map_allocate_space(vm_map_t* map, size_t length) */

void set_active_vm_map(vm_map_t *map);
vm_map_t *get_active_vm_map(pmap_type_t type);

void vm_map_init();
vm_map_t *vm_map_new(vm_map_type_t t, asid_t asid);
void vm_map_delete(vm_map_t *vm_map);

vm_map_entry_t *vm_map_find_entry(vm_map_t *vm_map, vm_addr_t vaddr);

void vm_map_protect(vm_map_t *map, vm_addr_t start, vm_addr_t end,
                    vm_prot_t prot);
vm_map_entry_t *vm_map_add_entry(vm_map_t *map, vm_addr_t start,
                                 vm_addr_t end, vm_prot_t prot);

void vm_map_dump(vm_map_t *vm_map);

void vm_page_fault(vm_map_t *map, vm_addr_t fault_addr, vm_prot_t fault_type);

#ifdef __cplusplus
}
#endif
#endif /* _VM_MAP_H */
