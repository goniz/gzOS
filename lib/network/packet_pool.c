#include <lib/malloc/malloc.h>
#include <assert.h>
#include <platform/drivers.h>
#include <stdlib.h>
#include <platform/interrupts.h>
#include <lib/mm/vm.h>
#include <lib/mm/physmem.h>
#include "packet_pool.h"

static MALLOC_DEFINE(mp_packet, "Packet buffer pool");

static int packet_pool_init(void)
{
    kmalloc_init(mp_packet);

    vm_page_t* page = pm_alloc(PACKET_POOL_SIZE / PAGESIZE);
    assert(page != NULL);

    kmalloc_add_arena(mp_packet, (void*)page->vaddr, PACKET_POOL_SIZE);
    return 0;
}

PacketView packet_pool_view_of_buffer(PacketBuffer packetBuffer, int offset, size_t size) {
    PacketView packetView;

    assert(packetBuffer.buffer != NULL);
    assert((packetBuffer.buffer_size - offset) >= size);

    if (0 == size) {
        size = packetBuffer.buffer_size - offset;
    }

    packetView.underlyingBuffer = packetBuffer;
    packetView.buffer = packetBuffer.buffer + offset;
    packetView.size = size;
    return packetView;
}

PacketBuffer packet_pool_view_to_buffer(PacketView view) {
    return view.underlyingBuffer;
}

PacketBuffer packet_pool_alloc(size_t size) {
    PacketBuffer buf;

    unsigned int isrMask = interrupts_disable();
    buf.buffer = kmalloc(mp_packet, size, M_ZERO | M_NOWAIT);
    interrupts_enable(isrMask);

    buf.buffer_capacity = buf.buffer_size = (buf.buffer == NULL ? 0 : size);
    return buf;
}

void packet_pool_free(PacketBuffer* pkt) {
    if (pkt->buffer) {
        unsigned int isrMask = interrupts_disable();
        kfree(mp_packet, pkt->buffer);
        interrupts_enable(isrMask);
    }
}

void packet_pool_free_underlying_buffer(PacketView* view) {
    if (view->underlyingBuffer.buffer) {
        packet_pool_free(&view->underlyingBuffer);
    }
}

DECLARE_DRIVER(packet_pool, packet_pool_init, STAGE_FIRST);