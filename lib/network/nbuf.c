#include <lib/malloc/malloc.h>
#include <assert.h>
#include <platform/drivers.h>
#include <stdlib.h>
#include <platform/interrupts.h>
#include <lib/mm/vm.h>
#include <lib/mm/physmem.h>
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

NetworkBuffer* nbuf_alloc(size_t size) {
    NetworkBuffer* nbuf = NULL;

    unsigned int isrMask = interrupts_disable();
    nbuf = kmalloc(mp_nbuf, sizeof(*nbuf), M_ZERO | M_NOWAIT);
    if (NULL == nbuf) {
        interrupts_enable(isrMask);
        return NULL;
    }

    nbuf->buffer.buffer = kmalloc(mp_data, size, M_NOWAIT);
    interrupts_enable(isrMask);

    nbuf->refcnt = 1;
    nbuf->buffer.buffer_capacity = (nbuf->buffer.buffer == NULL ? 0 : size);
    nbuf->buffer.buffer_size = 0;
    return nbuf;
}

void nbuf_free(NetworkBuffer* nbuf) {
    unsigned int isrMask = interrupts_disable();
    PacketBuffer* buf = &nbuf->buffer;

    nbuf->refcnt--;
    if (0 < nbuf->refcnt) {
        interrupts_enable(isrMask);
        return;
    }

    if (buf->buffer) {
        kfree(mp_data, buf->buffer);
    }

    kfree(mp_nbuf, nbuf);

    interrupts_enable(isrMask);
}

NetworkBuffer *nbuf_use(NetworkBuffer *nbuf) {
    unsigned int isrMask = interrupts_disable();
    nbuf->refcnt++;
    interrupts_enable(isrMask);

    return nbuf;
}

DECLARE_DRIVER(nbuf_pool, nbuf_pool_init, STAGE_FIRST);