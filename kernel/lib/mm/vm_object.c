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
#include <vm_object.h>
#include <lib/malloc/malloc.h>
#include <lib/primitives/align.h>
#include <assert.h>
#include <platform/kprintf.h>
#include <string.h>
#include "physmem.h"

static inline int vm_page_cmp(vm_page_t *a, vm_page_t *b) {
    if (a->vm_offset < b->vm_offset)
        return -1;
    return (int) (a->vm_offset - b->vm_offset);
}

RB_PROTOTYPE_STATIC(vm_object_tree, vm_page, obj.tree, vm_page_cmp);
RB_GENERATE(vm_object_tree, vm_page, obj.tree, vm_page_cmp);

static MALLOC_DEFINE(mpool, "vm_object memory pool");

void vm_object_init() {
    vm_page_t *pg = pm_alloc(2);
    kmalloc_init(mpool);
    kmalloc_add_arena(mpool, (void *) pg->vaddr, PG_SIZE(pg));
}

vm_object_t *vm_object_alloc() {
    vm_object_t *obj = kmalloc(mpool, sizeof(vm_object_t), M_ZERO);
    TAILQ_INIT(&obj->list);
    RB_INIT(&obj->tree);
    return obj;
}

void vm_object_free(vm_object_t *obj) {
    while (!TAILQ_EMPTY(&obj->list)) {
        vm_page_t *pg = TAILQ_FIRST(&obj->list);
        TAILQ_REMOVE(&obj->list, pg, obj.list);
        pm_free(pg);
    }

    memset(obj, 0, sizeof(*obj));
    kfree(mpool, obj);
}

vm_page_t *vm_object_find_page(vm_object_t *obj, vm_addr_t offset) {
    vm_page_t find = {.vm_offset = offset};
    return RB_FIND(vm_object_tree, &obj->tree, &find);
}

int vm_object_add_page(vm_object_t *obj, vm_page_t *page) {
    assert(is_aligned(page->vm_offset, PAGESIZE));
    /* For simplicity of implementation let's insert pages of size 1 only */
    assert(page->size == 1);

    if (!RB_INSERT(vm_object_tree, &obj->tree, page)) {
        obj->npages++;
        vm_page_t *next = RB_NEXT(vm_object_tree, &obj->tree, page);
        if (next)
            TAILQ_INSERT_BEFORE(next, page, obj.list);
        else
            TAILQ_INSERT_TAIL(&obj->list, page, obj.list);
        return 1;
    }

    return 0;
}

void vm_object_remove_page(vm_object_t *obj, vm_page_t *page) {
    TAILQ_REMOVE(&obj->list, page, obj.list);
    RB_REMOVE(vm_object_tree, &obj->tree, page);
    pm_free(page);
    obj->npages--;
}

void vm_map_object_dump(vm_object_t *obj) {
    vm_page_t *it;
    RB_FOREACH (it, vm_object_tree, &obj->tree) {
        kprintf("[vm_object] offset: %lu, size: %lu \n", it->vm_offset, it->size);
    }
}
