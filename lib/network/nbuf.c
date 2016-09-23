#include <lib/malloc/malloc.h>
#include <assert.h>
#include <platform/drivers.h>
#include <stdlib.h>
#include <platform/interrupts.h>
#include <lib/mm/vm.h>
#include <lib/mm/physmem.h>
#include <platform/kprintf.h>
#include "nbuf.h"

static MALLOC_DEFINE(mp_nbuf, "Network Buffer struct Pool");
static MALLOC_DEFINE(mp_data, "Network Buffer Data Pool");

static void init_memory_pool(malloc_pool_t* mp, size_t size)
{
    assert(size % PAGESIZE == 0);

    kmalloc_init(mp);

    vm_page_t* page = pm_alloc(size / PAGESIZE);
    assert(page != NULL);

    kmalloc_add_arena(mp, (void*)page->vaddr, size);
}

static int nbuf_pool_init(void)
{
    init_memory_pool(mp_nbuf, NetworkBufferControlPoolSize);
    init_memory_pool(mp_data, NetworkBufferDataPoolSize);
    return 0;
}

NetworkBuffer* nbuf_alloc_aligned(size_t size, int alignment) {
    NetworkBuffer* nbuf = NULL;

    unsigned int isrMask = interrupts_disable();
    nbuf = kmalloc(mp_nbuf, sizeof(*nbuf), M_ZERO | M_NOWAIT);
    if (NULL == nbuf) {
        interrupts_enable(isrMask);
        return NULL;
    }

    if (0 == alignment) {
        nbuf->buffer.buffer = kmalloc(mp_data, size, M_NOWAIT);
    } else {
        nbuf->buffer.buffer = kmemalign(mp_data, size, alignment, M_NOWAIT);
    }

    if (NULL == nbuf->buffer.buffer) {
        kfree(mp_nbuf, nbuf);
        interrupts_enable(isrMask);
        return NULL;
    }

    interrupts_enable(isrMask);

    nbuf->buffer.buffer_capacity = (nbuf->buffer.buffer == NULL ? 0 : size);
    nbuf->buffer.buffer_size = 0;
    return nbuf;
}

NetworkBuffer* nbuf_alloc(size_t size) {
    return nbuf_alloc_aligned(size, 0);
}

void nbuf_free(NetworkBuffer* nbuf) {
//    kprintf("nbuf: free(%p) from %p\n", nbuf, __builtin_return_address(0));
    unsigned int isrMask = interrupts_disable();

    if (nbuf->buffer.buffer) {
        kfree(mp_data, nbuf->buffer.buffer);
        nbuf->buffer.buffer = NULL;
    }

    kfree(mp_nbuf, nbuf);

    interrupts_enable(isrMask);
}

DECLARE_DRIVER(nbuf_pool, nbuf_pool_init, STAGE_FIRST);