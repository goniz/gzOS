//
// Created by gz on 6/25/16.
//

#include <lib/primitives/spinlock_mutex.h>
#include <lib/primitives/lock_guard.h>
#include <cstddef>
#include <platform/interrupts.h>
#include <lib/malloc/malloc.h>
#include <lib/primitives/interrupts_mutex.h>
#include <platform/panic.h>
#include <cstring>
#include <cassert>
#include <platform/kprintf.h>

static constexpr uint16_t AlignedMemBlockMagic = 0xAAFF;
struct AlignedMemBlock {
    uint16_t magic;
    uint16_t offset;
    uint8_t data[0];
};

static MALLOC_DEFINE(kmalloc_pool, "Global malloc pool");
static bool g_pool_initialized = false;

extern "C"
void malloc_init(void* start, size_t size)
{
    kmalloc_init(kmalloc_pool);
    kmalloc_add_arena(kmalloc_pool, start, size);
    g_pool_initialized = true;
}

extern "C"
void* malloc(size_t nbytes)
{
    if (!g_pool_initialized) {
        return nullptr;
    }

    InterruptsMutex intMutex;
    intMutex.lock();

    return kmalloc(kmalloc_pool, nbytes, M_ZERO);
}

extern "C"
void* _malloc_r(void* reent, size_t nbytes)
{
    return malloc(nbytes);
}

extern "C"
void free(void* ptr)
{
    if (nullptr == ptr) {
        return;
    }

    AlignedMemBlock* header = (AlignedMemBlock *)((uintptr_t)ptr - sizeof(AlignedMemBlock));
    if (AlignedMemBlockMagic == header->magic) {
        ptr = (void *) ((uintptr_t)ptr - header->offset);
    }

    InterruptsMutex intMutex;
    intMutex.lock();

    kfree(kmalloc_pool, ptr);
}

extern "C"
void _free_r(void* reent, void* ptr)
{
    free(ptr);
}

extern "C"
void* realloc(void* ptr, size_t size)
{
    InterruptsMutex intMutex;
    intMutex.lock();

    return krealloc(kmalloc_pool, ptr, size, M_ZERO);
}

extern "C"
void* _realloc_r(void* reent, void* ptr, size_t size)
{
    return realloc(ptr, size);
}

extern "C"
void* memalign(size_t size, int alignment)
{
    const size_t padded_size = size + alignment + sizeof(AlignedMemBlock);
    const uintptr_t ptr = (uintptr_t ) malloc(padded_size);
    if (0 == ptr) {
        return nullptr;
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

extern "C"
void* _memalign_r(void* reent, size_t size, int alignment)
{
	return memalign(size, alignment);
}
