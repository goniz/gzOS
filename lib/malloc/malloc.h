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

TAILQ_HEAD(ma_list, mem_arena);

typedef struct malloc_pool {
  SLIST_ENTRY(malloc_pool) mp_next;     /* Next in global chain. */
  uint32_t mp_magic;                    /* Detect programmer error. */
  const char *mp_desc;                  /* Printable type name. */
  struct ma_list mp_arena;              /* Queue of managed arenas. */
} malloc_pool_t;

/* Defines a local pool of memory for use by a subsystem. */
#define MALLOC_DEFINE(pool, desc)               \
    malloc_pool_t pool[1] = {                   \
        {{NULL}, MB_MAGIC, desc, {NULL, NULL}}  \
    };

#define MALLOC_DECLARE(pool) extern malloc_pool_t pool[1]

/* Flags to malloc */
#define M_WAITOK    0x0000 /* always returns memory block, but can sleep */
#define M_NOWAIT    0x0001 /* may return NULL, but cannot sleep */
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
