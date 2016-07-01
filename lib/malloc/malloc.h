#ifndef _MALLOC_H_
#define _MALLOC_H_

#include <stdint.h>
#include <stddef.h>
#include <lib/primitives/sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * General purpose kernel memory allocator.
 */

#define MB_MAGIC     0xC0DECAFE
#define MB_ALIGNMENT sizeof(uint64_t)

TAILQ_HEAD(mb_list, mem_block);
TAILQ_HEAD(mp_arena, mem_arena);

typedef struct mp_arena mp_arena_t;

typedef struct mem_block {
    uint32_t mb_magic;
    /* if overwritten report a memory corruption error */
    int32_t mb_size;   /* size > 0 => free, size < 0 => alloc'd */
    TAILQ_ENTRY(mem_block) mb_list;
    uint64_t mb_data[0];
} mem_block_t;

typedef struct mem_arena {
    TAILQ_ENTRY(mem_arena) ma_list;
    uint32_t ma_size;
    /* Size of all the blocks inside combined */
    uint16_t ma_flags;
    struct mb_list ma_freeblks;
    uint32_t ma_magic;
    /* Detect programmer error. */
    uint64_t ma_data[0];              /* For alignment */
} mem_arena_t;

typedef struct malloc_pool {
    SLIST_ENTRY(malloc_pool) mp_next;
    /* Next in global chain. */
    uint32_t mp_magic;
    /* Detect programmer error. */
    const char *mp_desc;
    /* Printable type name. */
    mp_arena_t mp_arena;              /* First managed arena. */
} malloc_pool_t;

/* Defines a local pool of memory for use by a subsystem. */
#define MALLOC_DEFINE(pool, desc) \
    malloc_pool_t pool[1] = {     \
        {{NULL}, MB_MAGIC, desc, {NULL, NULL} }  \
    };

#define MALLOC_DECLARE(pool)      \
    extern malloc_pool_t pool[1]

/* Flags to malloc */
#define M_WAITOK    0x0000 /* ignore for now */
#define M_NOWAIT    0x0001 /* ignore for now */
#define M_ZERO      0x0002 /* clear allocated block */

void kmalloc_init(malloc_pool_t *mp);

void kmalloc_add_arena(malloc_pool_t *mp, void *start, size_t size);

void* kmalloc(malloc_pool_t *mp, size_t size, uint16_t flags) __attribute__ ((warn_unused_result));

void* krealloc(malloc_pool_t* mp, void* ptr, size_t size, uint16_t flags) __attribute__ ((warn_unused_result));

void kfree(malloc_pool_t *mp, void *addr);

void kmalloc_dump(malloc_pool_t *mp);

#ifdef __cplusplus
};
#endif
#endif /* _MALLOC_H_ */
