#include <platform/drivers.h>
#include <lib/primitives/hashmap.h>
#include <lib/kernel/proc/Scheduler.h>
#include <cerrno>
#include "syscall.h"

extern "C" struct kernel_syscall syscall_table[];
extern "C" uint8_t __syscalls_start;
extern "C" uint8_t __syscalls_end;

static HashMap<uint32_t, kernel_syscall> _syscallsMap;

static int syscall_layer_init(void) {
    const size_t n_syscalls = ((&__syscalls_end - &__syscalls_start) / sizeof(struct kernel_syscall));

    for (size_t i = 0; i < n_syscalls; i++) {
        kernel_syscall& current = syscall_table[i];

//        kprintf("[syscall] loaded syscall %s @ %p\n", current.name, &current);
        _syscallsMap.put(current.number, current);
    }

    kprintf("[kernel] found %d system calls\n", n_syscalls);

    return 0;
}

DECLARE_DRIVER(syscall_layer, syscall_layer_init, STAGE_FIRST);

int syscall_handle(struct user_regs **regs, uint32_t syscallNumber, va_list args) {
#if SYSCALL_DEBUG == 1
    if (SYS_NR_YIELD != syscallNumber) {
        kprintf("syscall(%d, ...): ", syscallNumber);
    }
#endif

    auto* syscall = _syscallsMap.get(syscallNumber);
    if (!syscall) {
#if SYSCALL_DEBUG == 1
        if (SYS_NR_YIELD != syscallNumber) {
            kprintf("NotFound.\n");
        }
#endif
        return -ENOSYS;
    }

#if SYSCALL_DEBUG == 1
    if (SYS_NR_YIELD != syscallNumber) {
        kprintf("%s\n", (*syscall)->name);
    }
#endif
    return scheduler_syscall_handler(regs, syscall, args);
}