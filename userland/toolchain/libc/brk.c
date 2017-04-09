#include <stdint.h>
#include <stdatomic.h>
#include <syscall.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

extern char _end;

int brk(uintptr_t addr) {
    return syscall(SYS_NR_BRK, addr);
}

/* The statically held previous end of the heap, with its initialization. */
static uintptr_t _heap_ptr = (uintptr_t)&_end;
void* sbrk(ptrdiff_t increment) {
    void* ret = (void*) atomic_fetch_add(&_heap_ptr, increment);
    if (0 != brk(_heap_ptr)) {
        errno = ENOMEM;
        return NULL;
    }

    return ret;
}