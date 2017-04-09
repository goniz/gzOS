#include <lib/malloc/malloc.h>
#include <assert.h>
#include <platform/drivers.h>
#include <platform/interrupts.h>
#include <lib/mm/vm.h>
#include <lib/mm/physmem.h>
#include <platform/kprintf.h>
#include "lib/network/nbuf.h"

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

    nbuf->buffer.buffer_size = nbuf->buffer.buffer_capacity = (nbuf->buffer.buffer == NULL ? 0 : size);
    return nbuf;
}

NetworkBuffer* nbuf_alloc(size_t size) {
    NetworkBuffer* nbuf = nbuf_alloc_aligned(size, 0);

#ifdef NBUF_DEBUG
    kprintf("nbuf: alloc(%d) = %p from %p\n", size, nbuf, __builtin_return_address(0));
#endif

    return nbuf;
}

void nbuf_free(NetworkBuffer* nbuf) {
#ifdef NBUF_DEBUG
    kprintf("nbuf: free(%p) from %p\n", nbuf, __builtin_return_address(0));
#endif

    unsigned int isrMask = interrupts_disable();

    if (nbuf->buffer.buffer) {
        kfree(mp_data, nbuf->buffer.buffer);
        nbuf->buffer.buffer = NULL;
    }

    kfree(mp_nbuf, nbuf);

    interrupts_enable(isrMask);
}

NetworkBuffer* nbuf_clone(const NetworkBuffer *nbuf)
{
    NetworkBuffer* newNbuf = nbuf_alloc(nbuf_size(nbuf));
    uint8_t* newDataPtr = nbuf_data(newNbuf);
    memcpy(nbuf_data(newNbuf), nbuf_data(nbuf), nbuf_size(nbuf));

    nbuf_set_device(newNbuf, nbuf_device(nbuf));
    nbuf_set_size(newNbuf, nbuf_size(nbuf));

    if (nbuf->l2_offset) {
        nbuf_set_l2(newNbuf, newDataPtr + nbuf_offset(nbuf, nbuf->l2_offset));
    }

    if (nbuf->l3_offset) {
        nbuf_set_l3(newNbuf, newDataPtr + nbuf_offset(nbuf, nbuf->l3_offset), nbuf->l3_proto);
    }

    if (nbuf->l4_offset) {
        nbuf_set_l4(newNbuf, newDataPtr + nbuf_offset(nbuf, nbuf->l4_offset), nbuf->l4_proto);
    }

    return newNbuf;
}

NetworkBuffer* nbuf_clone_offset_nometadata(const NetworkBuffer* nbuf, size_t startOffset)
{
    if (startOffset >= nbuf_size(nbuf)) {
        return NULL;
    }

    void* nbuf_offset = (uint8_t*)nbuf_data(nbuf) + startOffset;
    size_t size_to_clone = nbuf_size_from(nbuf, nbuf_offset);
    NetworkBuffer* newNbuf = nbuf_alloc(size_to_clone);
    memcpy(nbuf_data(newNbuf), nbuf_offset, size_to_clone);

    nbuf_set_size(newNbuf, size_to_clone);

    return newNbuf;
}

DECLARE_DRIVER(nbuf_pool, nbuf_pool_init, STAGE_FIRST);