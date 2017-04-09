#include <lib/primitives/spinlock_mutex.h>
#include <platform/sbrk.h>
#include <platform/panic.h>
#include <cassert>
#include <cstring>
#include <lib/primitives/align.h>
#include <lib/mm/vm.h>

extern "C" char _end; // Defined by the linker

struct _sbrk_struct {
    void *ptr;     /* Pointer to the end of kernel's bss. */
    void *end;     /* Limit for the end of kernel's bss. */
    spinlock_mutex lock;
    bool shutdown;
};

static _sbrk_struct _sbrk{&_end, &_end + roundup(5 * 1024 * 1024, PAGESIZE), {}, false};

extern "C"
void kernel_brk(void *addr)
{
    if (_sbrk.shutdown) {
        panic("Trying to use kernel_brk after it's been shutdown!");
    }

    _sbrk.lock.lock();
//    void *ptr = _sbrk.ptr;
    addr = (void *)((intptr_t)addr & -sizeof(uint64_t));
    assert((intptr_t)&_end <= (intptr_t)addr);
    assert((intptr_t)addr <= (intptr_t)_sbrk.end);
    _sbrk.ptr = addr;
    _sbrk.lock.unlock();

//    if (addr > ptr) {
//        memset(ptr, 0, (intptr_t) addr - (intptr_t) ptr);
//    }
}

extern "C"
void* kernel_sbrk(size_t size)
{
    if (_sbrk.shutdown) {
        panic("Trying to use kernel_sbrk after it's been shutdown!");
    }

    _sbrk.lock.lock();
    void *ptr = _sbrk.ptr;
    size = roundup(size, sizeof(uint64_t));
    assert(((uintptr_t)ptr + size) <= (uintptr_t)_sbrk.end);
    _sbrk.ptr = (void*)((uintptr_t)_sbrk.ptr + size);
    _sbrk.lock.unlock();

//    memset(ptr, 0, size);
    return ptr;
}

extern "C"
void* kernel_sbrk_shutdown(void)
{
    assert(!_sbrk.shutdown);

    _sbrk.lock.lock();
    _sbrk.end = align(_sbrk.ptr, PAGESIZE);
    _sbrk.shutdown = true;
    _sbrk.lock.unlock();

    return _sbrk.end;
}
