#include <lib/primitives/SpinlockMutex.h>
#include <lib/primitives/LockGuard.h>
#include <cstddef>
#include <platform/interrupts.h>
#include <lib/malloc/malloc.h>
#include <lib/primitives/InterruptsMutex.h>
#include <platform/panic.h>
#include <cstring>
#include <cassert>
#include <platform/kprintf.h>

static MALLOC_DEFINE(kmalloc_pool, "Global malloc pool");
static bool g_pool_initialized = false;

extern "C"
void malloc_init(void* start, size_t size)
{
    kprintf("[malloc] initializing kernel malloc pool of size %d at %p\n", size, start);

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
    if (!g_pool_initialized) {
        return nullptr;
    }

    InterruptsMutex intMutex;
    intMutex.lock();

    return kmemalign(kmalloc_pool, size, alignment, M_NOWAIT);
}

extern "C"
void* _memalign_r(void* reent, size_t size, int alignment)
{
	return memalign(size, alignment);
}
