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

#include <malloc.h>
#include <pmap.h>
#include <vm_pager.h>
#include <vm_object.h>
#include <vm_map.h>
#include <lib/malloc/malloc.h>
#include <lib/primitives/align.h>
#include <assert.h>
#include <platform/kprintf.h>
#include <platform/panic.h>
#include "physmem.h"

static vm_map_t *active_vm_map[2];

void set_active_vm_map(vm_map_t *map) {
    pmap_type_t type = map->pmap.type;
    active_vm_map[type] = map;
    set_active_pmap(&map->pmap);
}

vm_map_t *get_active_vm_map(pmap_type_t type) {
    return active_vm_map[type];
}

static inline int vm_map_entry_cmp(vm_map_entry_t *a, vm_map_entry_t *b) {
    if (a->start < b->start)
        return -1;
    return (int) (a->start - b->start);
}

SPLAY_PROTOTYPE(vm_map_tree, vm_map_entry, map_tree, vm_map_entry_cmp);
SPLAY_GENERATE(vm_map_tree, vm_map_entry, map_tree, vm_map_entry_cmp);

static MALLOC_DEFINE(mpool, "vm_map memory pool");

void vm_map_init() {
    vm_page_t *pg = pm_alloc(2);
    kmalloc_init(mpool);
    kmalloc_add_arena(mpool, (void *) pg->vaddr, PG_SIZE(pg));
    vm_map_t *map = vm_map_new((vm_map_type_t) PMAP_KERNEL, 0);
    set_active_vm_map(map);
}

vm_map_t *vm_map_new(vm_map_type_t type, asid_t asid) {
    vm_map_t *map = kmalloc(mpool, sizeof(vm_map_t), M_ZERO);

    TAILQ_INIT(&map->list);
    SPLAY_INIT(&map->tree);
    pmap_setup(&map->pmap, (pmap_type_t) type, asid);
    map->nentries = 0;
    return map;
}

static int vm_map_insert_entry(vm_map_t *vm_map, vm_map_entry_t *entry) {
    if (!SPLAY_INSERT(vm_map_tree, &vm_map->tree, entry)) {
        vm_map_entry_t *next = SPLAY_NEXT(vm_map_tree, &vm_map->tree, entry);
        if (next)
            TAILQ_INSERT_BEFORE(next, entry, map_list);
        else
            TAILQ_INSERT_TAIL(&vm_map->list, entry, map_list);
        vm_map->nentries++;
        return 1;
    }
    return 0;
}

vm_map_entry_t *vm_map_find_entry(vm_map_t *vm_map, vm_addr_t vaddr) {
    vm_map_entry_t *etr_it;
    TAILQ_FOREACH (etr_it, &vm_map->list, map_list)
        if (etr_it->start <= vaddr && vaddr < etr_it->end)
            return etr_it;
    return NULL;
}

static void vm_map_remove_entry(vm_map_t *vm_map, vm_map_entry_t *entry) {
    vm_map->nentries--;
    vm_object_free(entry->object);
    TAILQ_REMOVE(&vm_map->list, entry, map_list);
    kfree(mpool, entry);
}

void vm_map_delete(vm_map_t *map) {
    while (map->nentries > 0)
        vm_map_remove_entry(map, TAILQ_FIRST(&map->list));
    kfree(mpool, map);
}

vm_map_entry_t *vm_map_add_entry(vm_map_t *map, vm_addr_t start,
                                 vm_addr_t end, vm_prot_t prot) {
    assert(start >= map->pmap.start);
    assert(end <= map->pmap.end);
    assert(is_aligned(start, PAGESIZE));
    assert(is_aligned(end, PAGESIZE));

#if 0
    assert(vm_map_find_entry(map, start) == NULL);
  assert(vm_map_find_entry(map, end) == NULL);
#endif

    vm_map_entry_t *entry = kmalloc(mpool, sizeof(vm_map_entry_t), M_ZERO);

    entry->start = start;
    entry->end = end;
    entry->prot = prot;

    pmap_map(&map->pmap, start, 0, (end - start) / PAGESIZE, VM_PROT_NONE);

    vm_map_insert_entry(map, entry);
    return entry;
}

/* TODO: not implemented */
void vm_map_protect(vm_map_t *map, vm_addr_t start, vm_addr_t end,
                    vm_prot_t prot) {
}

void vm_map_dump(vm_map_t *map) {
    vm_map_entry_t *it;
    kprintf("[vm_map] Virtual memory map (%08lx - %08lx):\n",
            map->pmap.start, map->pmap.end);
    TAILQ_FOREACH (it, &map->list, map_list) {
        kprintf("[vm_map] * %08lx - %08lx [%c%c%c]\n",
                it->start, it->end,
                (it->prot & VM_PROT_READ) ? 'r' : '-',
                (it->prot & VM_PROT_WRITE) ? 'w' : '-',
                (it->prot & VM_PROT_EXEC) ? 'x' : '-');
        vm_map_object_dump(it->object);
    }
}

void vm_page_fault(vm_map_t *map, vm_addr_t fault_addr, vm_prot_t fault_type) {
    vm_map_entry_t *entry;

    kprintf("Page fault!\n");

    if (!(entry = vm_map_find_entry(map, fault_addr)))
        panic("Tried to access unmapped memory region: 0x%08lx!\n", fault_addr);

    if (entry->prot == VM_PROT_NONE)
        panic("Cannot access to address: 0x%08lx\n", fault_addr);

    if (!(entry->prot & VM_PROT_WRITE) && (fault_type == VM_PROT_WRITE))
        panic("Cannot write to address: 0x%08lx\n", fault_addr);

    if (!(entry->prot & VM_PROT_READ) && (fault_type == VM_PROT_READ))
        panic("Cannot read from address: 0x%08lx\n", fault_addr);

    assert(entry->start <= fault_addr && fault_addr < entry->end);

    vm_object_t *obj = entry->object;

    assert(obj != NULL);

    vm_addr_t fault_page = fault_addr & -PAGESIZE;
    vm_addr_t offset = fault_page - entry->start;
    vm_page_t *accessed_page = vm_object_find_page(entry->object, offset);

    if (accessed_page == NULL) {
        accessed_page = obj->pgr->pgr_fault(obj, fault_page, offset, fault_type);
    }

    pmap_map(&map->pmap, fault_addr, accessed_page->paddr, 1, (vm_prot_t) entry->prot);
}
