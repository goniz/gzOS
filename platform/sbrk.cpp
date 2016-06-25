//
// Created by gz on 6/25/16.
//

#include <lib/primitives/spinlock_mutex.h>
#include <platform/malta/vm.h>
#include <platform/sbrk.h>
#include <platform/panic.h>
#include <lib/primitives/lock_guard.h>
#include <cassert>
#include <cstring>
#include <lib/primitives/align.h>

extern "C" char _end; // Defined by the linker

static struct {
    void *ptr;     /* Pointer to the end of kernel's bss. */
    void *end;     /* Limit for the end of kernel's bss. */
    spinlock_mutex lock;
    bool shutdown;
} sbrk = { &_end, &_end + 1024 * PAGESIZE, {}, false };

extern "C"
void kernel_brk(void *addr)
{
    if (sbrk.shutdown) {
        panic("Trying to use kernel_brk after it's been shutdown!");
    }

    sbrk.lock.lock();
    void *ptr = sbrk.ptr;
    addr = (void *)((intptr_t)addr & -sizeof(uint64_t));
    assert((intptr_t)&_end <= (intptr_t)addr);
    assert((intptr_t)addr <= (intptr_t)sbrk.end);
    sbrk.ptr = addr;
    sbrk.lock.unlock();

    if (addr > ptr)
        memset(ptr, 0, (intptr_t)addr - (intptr_t)ptr);
}

extern "C"
void* kernel_sbrk(size_t size)
{
    if (sbrk.shutdown) {
        panic("Trying to use kernel_sbrk after it's been shutdown!");
    }

    sbrk.lock.lock();
    void *ptr = sbrk.ptr;
    size = roundup(size, sizeof(uint64_t));
    assert((uintptr_t)ptr + size <= (uintptr_t)sbrk.end);
    sbrk.ptr = (void*)((uintptr_t)sbrk.ptr + size);
    sbrk.lock.unlock();

    memset(ptr, 0, size);
    return ptr;
}

extern "C"
void* kernel_sbrk_shutdown(void)
{
    assert(!sbrk.shutdown);

    sbrk.lock.lock();
    sbrk.end = align(sbrk.ptr, PAGESIZE);
    sbrk.shutdown = true;
    sbrk.lock.unlock();

    return sbrk.end;
}
