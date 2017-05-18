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
#include <platform/malta/tlb.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <platform/cpu.h>
#include "physmem.h"
#include "pmap.h"

static vm_map_t kspace;
static vm_map_t* uspace;

vm_map_t* vm_map_activate(vm_map_t* map) {
    int irq = interrupts_disable();

    vm_map_t* old = uspace;
    uspace = map;
    pmap_activate(map ? map->pmap : NULL);

    interrupts_enable(irq);
    return old;
}

vm_map_t* get_user_vm_map() {
    return uspace;
}

vm_map_t* get_kernel_vm_map() {
    return &kspace;
}

static bool in_range(vm_map_t* map, vm_addr_t addr) {
    /* No need to enter RWlock, the pmap field is const. */
    return map && map->pmap->start <= addr && addr < map->pmap->end;
}

vm_map_t* get_active_vm_map_by_addr(vm_addr_t addr) {
    if (in_range(get_user_vm_map(), addr)) {
        return get_user_vm_map();
    }

    if (in_range(get_kernel_vm_map(), addr)) {
        return get_kernel_vm_map();
    }

    return NULL;
}

static inline int vm_map_entry_cmp(vm_map_entry_t* a, vm_map_entry_t* b) {
    if (a->start < b->start)
        return -1;
    return (int) (a->start - b->start);
}

SPLAY_PROTOTYPE(vm_map_tree, vm_map_entry, map_tree, vm_map_entry_cmp);

SPLAY_GENERATE(vm_map_tree, vm_map_entry, map_tree, vm_map_entry_cmp);

static void vm_map_setup(vm_map_t* map) {
    TAILQ_INIT(&map->list);
    SPLAY_INIT(&map->tree);
}

static MALLOC_DEFINE(mpool, "vm_map memory pool");

void vm_map_init() {
    vm_page_t* pg = pm_alloc(2);
    kmalloc_init(mpool);
    kmalloc_add_arena(mpool, (void*) PG_VADDR_START(pg), PG_SIZE(pg));

    vm_map_setup(&kspace);
    *((pmap_t**) (&kspace.pmap)) = get_kernel_pmap();
}

vm_map_t* vm_map_new() {
    vm_map_t* map = kmalloc(mpool, sizeof(vm_map_t), M_ZERO);

    vm_map_setup(map);
    *((pmap_t**) &map->pmap) = pmap_new();
    return map;
}

static int vm_map_insert_entry(vm_map_t* vm_map, vm_map_entry_t* entry) {
    int irq = interrupts_disable();
    if (!SPLAY_INSERT(vm_map_tree, &vm_map->tree, entry)) {
        vm_map_entry_t* next = SPLAY_NEXT(vm_map_tree, &vm_map->tree, entry);
        if (next) {
            TAILQ_INSERT_BEFORE(next, entry, map_list);
        } else {
            TAILQ_INSERT_TAIL(&vm_map->list, entry, map_list);
        }
        vm_map->nentries++;
        interrupts_enable(irq);
        return 1;
    }

    interrupts_enable(irq);
    return 0;
}

vm_map_entry_t* vm_map_find_entry(vm_map_t* vm_map, vm_addr_t vaddr) {
    int irq = interrupts_disable();

    vm_map_entry_t* etr_it;
    TAILQ_FOREACH (etr_it, &vm_map->list, map_list) {
        if (etr_it->start <= vaddr && vaddr < etr_it->end) {
            interrupts_enable(irq);
            return etr_it;
        }
    }

    interrupts_enable(irq);
    return NULL;
}

void vm_map_remove_entry(vm_map_t* vm_map, vm_map_entry_t* entry) {
    int irq = interrupts_disable();

    vm_map->nentries--;
    vm_object_free(entry->object);
    TAILQ_REMOVE(&vm_map->list, entry, map_list);
    memset(entry, 0, sizeof(*entry));
    kfree(mpool, entry);

    interrupts_enable(irq);
}

void vm_map_delete(vm_map_t* map) {
    int irq = interrupts_disable();

    while (map->nentries > 0) {
        vm_map_remove_entry(map, TAILQ_FIRST(&map->list));
    }

    pmap_delete(map->pmap);

    memset(map, 0, sizeof(*map));
    kfree(mpool, map);

    interrupts_enable(irq);
}

vm_map_entry_t* vm_map_add_entry(vm_map_t* map, vm_addr_t start,
                                 vm_addr_t end, vm_prot_t prot) {
    assert(start >= map->pmap->start);
    assert(end <= map->pmap->end);
    assert(is_aligned(start, PAGESIZE));
    assert(is_aligned(end, PAGESIZE));

#if 0
    assert(vm_map_find_entry(map, start) == NULL);
    assert(vm_map_find_entry(map, end) == NULL);
#endif

    int irq = interrupts_disable();

    vm_map_entry_t* entry = kmalloc(mpool, sizeof(vm_map_entry_t), M_ZERO);

    entry->start = start;
    entry->end = end;
    entry->prot = prot;

    vm_map_insert_entry(map, entry);

    interrupts_enable(irq);
    return entry;
}

/* TODO: not implemented */
void vm_map_protect(vm_map_t* map, vm_addr_t start, vm_addr_t end, vm_prot_t prot) {
}

int vm_map_findspace_nolock(vm_map_t* map, vm_addr_t start, size_t length, vm_addr_t /*out*/ * addr) {
    assert(is_aligned(start, PAGESIZE));
    assert(is_aligned(length, PAGESIZE));

    int irq = interrupts_disable();

    /* Bounds check */
    if (start < map->pmap->start) {
        start = map->pmap->start;
    }

    if (start + length > map->pmap->end) {
        goto fail;
    }

    /* Entire space free? */
    if (TAILQ_EMPTY(&map->list)) {
        goto found;
    }

    /* Is enought space before the first entry in the map? */
    vm_map_entry_t* first = TAILQ_FIRST(&map->list);
    if (start + length <= first->start) {
        goto found;
    }

    /* Browse available gaps. */
    vm_map_entry_t* it;
    TAILQ_FOREACH (it, &map->list, map_list) {
        vm_map_entry_t* next = TAILQ_NEXT(it, map_list);
        vm_addr_t gap_start = it->end;
        vm_addr_t gap_end = next ? next->start : map->pmap->end;

        /* Move start address forward if it points inside allocated space. */
        if (start < gap_start) {
            start = gap_start;
        }

        /* Will we fit inside this gap? */
        if (start + length <= gap_end) {
            goto found;
        }
    }

fail:
    /* Failed to find free space. */
    interrupts_enable(irq);
    return -ENOMEM;

found:
    *addr = start;
    interrupts_enable(irq);
    return 0;
}

int vm_map_findspace(vm_map_t* map, vm_addr_t start, size_t length, vm_addr_t /*out*/ * addr) {
    // NOTE: this _nolock is actually locking..
    return vm_map_findspace_nolock(map, start, length, addr);
}

int vm_map_resize(vm_map_t* map, vm_map_entry_t* entry, vm_addr_t new_end) {
    assert(is_aligned(new_end, PAGESIZE));
    int irq = interrupts_disable();

    /* TODO: As for now, we are unable to decrease the size of an entry, because
       it would require unmapping physical pages, which in turn should clean
       TLB. This is not implemented yet, and therefore shrinking an entry
       immediately leads to very confusing behavior, as the vm_map and TLB entries
       do not match. */
    assert(new_end >= entry->end);

    if (new_end > entry->end) {
        /* Expanding entry */
        vm_map_entry_t* next = TAILQ_NEXT(entry, map_list);
        vm_addr_t gap_end = next ? next->start : map->pmap->end;
        if (new_end > gap_end) {
            interrupts_enable(irq);
            return 0;
        }
    } else {
        /* Shrinking entry */
        if (new_end < entry->start) {
            interrupts_enable(irq);
            return 0;
        }
        /* TODO: Invalidate tlb? */
    }

    /* Note that neither tailq nor splay tree require updating. */
    entry->end = new_end;

    interrupts_enable(irq);
    return 1;
}

void vm_map_dump(vm_map_t* map) {
    vm_map_entry_t* it;

    int irq = interrupts_disable();

    kprintf("[vm_map] Virtual memory map (%08lx - %08lx):\n",
            map->pmap->start,
            map->pmap->end);

    TAILQ_FOREACH (it, &map->list, map_list) {
        kprintf("[vm_map] * %08lx - %08lx [%c%c%c]\n", it->start, it->end,
                (it->prot & VM_PROT_READ) ? 'r' : '-',
                (it->prot & VM_PROT_WRITE) ? 'w' : '-',
                (it->prot & VM_PROT_EXEC) ? 'x' : '-');
        vm_map_object_dump(it->object);
    }

    interrupts_enable(irq);
}

/* This entire function is a nasty hack, but we'll live with it until proper COW
   is implemented. */
vm_map_t* vm_map_clone(vm_map_t* map) {
    vm_map_t* orig_current_map = get_user_vm_map();
    vm_map_t* newmap = vm_map_new();

    int irq = interrupts_disable();

    /* Temporarily switch to the new map, so that we may write contents. Note that
       it's okay if we get preempted - the working vm map will be restored on
       context switch. */
    vm_map_activate(newmap);

    vm_map_entry_t* it;
    TAILQ_FOREACH (it, &map->list, map_list) {
        vm_map_entry_t* entry = vm_map_add_entry(newmap, it->start, it->end, it->prot);
        entry->object = default_pager->pgr_alloc();

        vm_page_t* page;
        TAILQ_FOREACH (page, &it->object->list, obj.list) {
            char* dst = (char*) it->start + page->vm_offset;
            char* src = (char*) platform_phy_to_virt(page->paddr);
            memcpy(dst, src, PG_SIZE(page));
        }
    }

    /* Return to original vm map. */
    vm_map_activate(orig_current_map);

    interrupts_enable(irq);
    return newmap;
}

int vm_page_fault(vm_map_t* map, vm_addr_t fault_addr, vm_prot_t fault_type) {
    vm_map_entry_t* entry = NULL;

#if TLBDEBUG == 1
    kprintf("vm_page_fault map %p, asid %d addr %p\n", map, map->pmap->asid, (void*)fault_addr);
    vm_map_dump(map);
#endif

    if (!(entry = vm_map_find_entry(map, fault_addr))) {
        goto segfault;
    }

    if (entry->prot == VM_PROT_NONE) {
        goto segfault;
    }

    if (!(entry->prot & VM_PROT_WRITE) && (fault_type == VM_PROT_WRITE)) {
        goto segfault;
    }

    if (!(entry->prot & VM_PROT_READ) && (fault_type == VM_PROT_READ)) {
        goto segfault;
    }

    assert(entry->start <= fault_addr && fault_addr < entry->end);

    vm_object_t* obj = entry->object;

    assert(obj != NULL);

    vm_addr_t fault_page = fault_addr & -PAGESIZE;
    vm_addr_t offset = fault_page - entry->start;
    vm_page_t* frame = vm_object_find_page(entry->object, offset);

    if (frame == NULL) {
        frame = obj->pgr->pgr_fault(obj, fault_page, offset, fault_type);
    }

    const vm_addr_t start = fault_page;
    const vm_addr_t end = fault_page + PAGESIZE;
    pmap_map(map->pmap, start, end, frame->paddr, (vm_prot_t) entry->prot);

#if TLBDEBUG == 1
    kprintf("vm_page_fault DONE map %p, asid %d addr %p\n", map, map->pmap->asid, (void*)fault_addr);
    vm_map_dump(map);
#endif

    return 1;

    segfault:

#if TLBDEBUG == 1
    vm_map_dump(map);
#endif

    return 0;
}
