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

#include <lib/primitives/sys/queue.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <lib/primitives/align.h>
#include <platform/panic.h>
#include <platform/kprintf.h>
#include "malloc.h"

/*
  TODO:
  - use the mp_next field of malloc_pool
*/

TAILQ_HEAD(mb_list, mem_block);

typedef struct mem_block {
    uint32_t mb_magic; /* if overwritten report a memory corruption error */
    int32_t mb_size;   /* size > 0 => free, size < 0 => alloc'd */
    TAILQ_ENTRY(mem_block) mb_list;
    uint64_t mb_data[0];
} mem_block_t;

typedef struct mem_arena {
    TAILQ_ENTRY(mem_arena) ma_list;
    uint32_t ma_size;                 /* Size of all the blocks inside combined */
    uint16_t ma_flags;
    struct mb_list ma_freeblks;
    uint32_t ma_magic;                /* Detect programmer error. */
    uint64_t ma_data[0];              /* For alignment */
} mem_arena_t;


static const uint16_t AlignedMemBlockMagic = 0xAAFF;
typedef struct {
    uint16_t magic;
    uint16_t offset;
    uint8_t data[0];
} AlignedMemBlock;

static inline mem_block_t *mb_next(mem_block_t *block) {
    return (void *) block + abs(block->mb_size) + sizeof(mem_block_t);
}

static void merge_right(struct mb_list *ma_freeblks, mem_block_t *mb) {
    mem_block_t *next = TAILQ_NEXT(mb, mb_list);

    if (!next)
        return;

    char *mb_ptr = (char *) mb;
    if (mb_ptr + mb->mb_size + sizeof(mem_block_t) == (char *) next) {
        TAILQ_REMOVE(ma_freeblks, next, mb_list);
        mb->mb_size = mb->mb_size + next->mb_size + sizeof(mem_block_t);
    }
}

static void add_free_memory_block(mem_arena_t *ma, mem_block_t *mb,
                                  size_t total_size) {
    memset(mb, 0, sizeof(mem_block_t));
    mb->mb_magic = MB_MAGIC;
    mb->mb_size = total_size - sizeof(mem_block_t);

    // If it's the first block, we simply add it.
    if (TAILQ_EMPTY(&ma->ma_freeblks)) {
        TAILQ_INSERT_HEAD(&ma->ma_freeblks, mb, mb_list);
        return;
    }

    // It's not the first block, so we insert it in a sorted fashion.
    mem_block_t *current = NULL;
    mem_block_t *best_so_far = NULL;  /* mb can be inserted after this entry. */

    TAILQ_FOREACH(current, &ma->ma_freeblks, mb_list) {
        if (current < mb)
            best_so_far = current;
    }

    if (!best_so_far) {
        TAILQ_INSERT_HEAD(&ma->ma_freeblks, mb, mb_list);
        merge_right(&ma->ma_freeblks, mb);
    } else {
        TAILQ_INSERT_AFTER(&ma->ma_freeblks, best_so_far, mb, mb_list);
        merge_right(&ma->ma_freeblks, mb);
        merge_right(&ma->ma_freeblks, best_so_far);
    }
}

void kmalloc_init(malloc_pool_t *mp) {
    assert(NULL != mp);
    memset(mp, 0, sizeof(*mp));

    mp->mp_magic = MB_MAGIC;
    mp->mp_next.sle_next = NULL;
    TAILQ_INIT(&mp->mp_arena);
}

void kmalloc_set_description(malloc_pool_t *mp, const char *desc) {
    assert(NULL != mp);
    mp->mp_desc = desc;
}

void kmalloc_add_arena(malloc_pool_t *mp, void *start, size_t arena_size) {
//    kprintf("%s->kmalloc_add_arena(%p, %p, %d)\n", mp->mp_desc, mp, start, arena_size);

    if (arena_size < sizeof(mem_arena_t))
        return;

    memset(start, 0, sizeof(mem_arena_t));
    mem_arena_t *ma = start;

    TAILQ_INSERT_HEAD(&mp->mp_arena, ma, ma_list);
    ma->ma_size = arena_size - sizeof(mem_arena_t);
    ma->ma_magic = MB_MAGIC;
    ma->ma_flags = 0;

    TAILQ_INIT(&ma->ma_freeblks);

    // Adding the first free block that covers all the remaining arena_size.
    mem_block_t *mb = (mem_block_t *) ((char *) ma + sizeof(mem_arena_t));
    size_t block_size = arena_size - sizeof(mem_arena_t);
    add_free_memory_block(ma, mb, block_size);
}

static mem_block_t *find_entry(struct mb_list *mb_list, size_t total_size) {
    mem_block_t *current = NULL;
    TAILQ_FOREACH(current, mb_list, mb_list) {
        assert(current->mb_magic == MB_MAGIC);
        if (current->mb_size >= total_size)
            return current;
    }
    return NULL;
}

static mem_block_t *
try_allocating_in_area(mem_arena_t *ma, size_t requested_size) {
    mem_block_t *mb = find_entry(&ma->ma_freeblks,
                                 requested_size + sizeof(mem_block_t));

    if (!mb) /* No entry has enough space. */
        return NULL;

    TAILQ_REMOVE(&ma->ma_freeblks, mb, mb_list);
    size_t total_size_left = mb->mb_size - requested_size;
    if (total_size_left > sizeof(mem_block_t)) {
        mb->mb_size = -requested_size;
        mem_block_t *new_mb = (mem_block_t *)
                ((char *) mb + requested_size + sizeof(mem_block_t));
        add_free_memory_block(ma, new_mb, total_size_left);
    }
    else
        mb->mb_size = -mb->mb_size;

    return mb;
}

void *kmalloc(malloc_pool_t *mp, size_t size, uint16_t flags) {
    if (0 >= size) {
        return NULL;
    }

    size_t size_aligned = align(size, MB_ALIGNMENT);

//    kprintf("%s->kmalloc(%p, %d)\n", mp->mp_desc, mp, size);

    /* Search for the first area in the list that has enough space. */
    mem_arena_t *current = NULL;
    TAILQ_FOREACH(current, &mp->mp_arena, ma_list) {
        if (current->ma_magic != MB_MAGIC) {
            kmalloc_dump(mp);
            panic("Memory corruption detected!");
        }

        mem_block_t *mb = try_allocating_in_area(current, size_aligned);
        if (mb) {
            if (flags & M_ZERO) {
                memset(mb->mb_data, 0, size);
            }

            return mb->mb_data;
        }
    }

    if (flags & M_NOWAIT) {
        return NULL;
    }

	kmalloc_dump(mp);
    panic("memory exhausted in '%s' size requested %d", mp->mp_desc, size);;
}

void kfree(malloc_pool_t *mp, void *addr) {
    AlignedMemBlock* header = (AlignedMemBlock *)((uintptr_t)addr - sizeof(AlignedMemBlock));
    if (AlignedMemBlockMagic == header->magic) {
        addr = (void *) ((uintptr_t)addr - header->offset);
    }

    mem_block_t *mb = (mem_block_t *) (((char *) addr) - sizeof(mem_block_t));
    if (mb->mb_magic != MB_MAGIC ||
        mp->mp_magic != MB_MAGIC ||
        mb->mb_size >= 0) {
        kmalloc_dump(mp);
        panic("Memory corruption detected!");
    }

//    kprintf("%s->kfree(%p, %p)\n", mp->mp_desc, mp, addr);

    mem_arena_t *current = NULL;
    TAILQ_FOREACH(current, &mp->mp_arena, ma_list) {
        char *start = ((char *) current) + sizeof(mem_arena_t);
        if ((char *) addr >= start && (char *) addr < start + current->ma_size)
            add_free_memory_block(current, mb, abs(mb->mb_size) + sizeof(mem_block_t));
    }
}

void *krealloc(malloc_pool_t *mp, void *ptr, size_t size, uint16_t flags) {
    mem_block_t *mb = (mem_block_t *) (((char *) ptr) - sizeof(mem_block_t));

    if (mb->mb_magic != MB_MAGIC ||
        mp->mp_magic != MB_MAGIC ||
        mb->mb_size >= 0) {
        kmalloc_dump(mp);
        panic("Memory corruption detected!");
    }

    if (0 == size) {
        return NULL;
    }

    void *new_ptr = kmalloc(mp, size, flags);
    if (NULL == new_ptr) {
        return NULL;
    }

    memcpy(new_ptr, ptr, (size_t) abs(mb->mb_size));
    kfree(mp, ptr);
    return new_ptr;
}

void* kmemalign(malloc_pool_t *mp, size_t size, int alignment, uint16_t flags)
{
    const size_t padded_size = size + alignment + sizeof(AlignedMemBlock);
    const uintptr_t ptr = (uintptr_t ) kmalloc(mp, padded_size, flags);
    if (0 == ptr) {
        return NULL;
    }

    // leave it be if its alraedy aligned
    if (0 == (ptr % alignment)) {
        return (void *) ptr;
    }

    const uintptr_t end_ptr = ptr + padded_size;
    const uintptr_t plus_block = (ptr + sizeof(AlignedMemBlock));
    const uintptr_t aligned_ptr = ((plus_block + (alignment - 1)) & ~(alignment - 1));

    // make sure that we have enough room for the actual buffer size
    assert((end_ptr - aligned_ptr) >= size);

    AlignedMemBlock* header = (AlignedMemBlock *)(aligned_ptr - sizeof(AlignedMemBlock));
    header->magic = AlignedMemBlockMagic;
    header->offset = (uint16_t)(aligned_ptr - ptr);

    return (void *) aligned_ptr;
}

void kmalloc_dump(malloc_pool_t *mp) {
    mem_arena_t *arena = NULL;
    kprintf("[kmalloc] malloc_pool '%s' at %p:\n", mp->mp_desc, mp);
    TAILQ_FOREACH(arena, &mp->mp_arena, ma_list) {
        mem_block_t *block = (void *) arena->ma_data;
        mem_block_t *end = (void *) arena->ma_data + arena->ma_size;

        kprintf("[kmalloc]  malloc_arena %p – %p:\n", block, end);

        while (block < end) {
            assert(block->mb_magic == MB_MAGIC);
            kprintf("[kmalloc]   %c %p %d\n", (block->mb_size > 0) ? 'F' : 'U',
                    block, (unsigned) abs(block->mb_size));
            block = mb_next(block);
        }
    }
}


