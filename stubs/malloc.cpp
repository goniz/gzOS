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
void* sbrk(size_t size)
{
    if (!g_pool_initialized) {
        panic("sbrk called before malloc init");
    }

    InterruptsMutex intMutex;
    intMutex.lock();

    return kmalloc(kmalloc_pool, size, M_ZERO);
}

//extern "C"
//void* malloc(size_t nbytes)
//{
//    if (!g_pool_initialized) {
//        return nullptr;
//    }
//
//    InterruptsMutex intMutex;
//    intMutex.lock();
//
//    return kmalloc(kmalloc_pool, nbytes, M_ZERO);
//}
//
//extern "C"
//void* _malloc_r(void *reent, size_t nbytes)
//{
//    return malloc(nbytes);
//}
//
//extern "C"
//void free(void* ptr)
//{
//    if (nullptr == ptr) {
//        return;
//    }
//
//    InterruptsMutex intMutex;
//    intMutex.lock();
//
//    kfree(kmalloc_pool, ptr);
//}
//
//extern "C"
//void _free_r(void *reent, void* ptr)
//{
//    return free(ptr);
//}
//
//extern "C"
//void* realloc(void* ptr, size_t size)
//{
//    return ptr;
//}