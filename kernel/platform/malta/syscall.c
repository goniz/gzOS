#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <lib/syscall/syscall.h>
#include <lib/kernel/scheduler.h>
#include <sys/cdefs.h>
#include "interrupts.h"

long int syscall(long int number, ...)
{
    // NOTE: apparently the vaargs things needs a real stack frame to work (??)
    __unused volatile char tmp[8] = {0};
    va_list arg;

//    if (platform_is_irq_context()) {
//        panic("System calls cannot be used in IRQ context.");
//        kputs("warning: System calls cannot be used in IRQ context.\n");
//    }

    va_start(arg, number);
    asm volatile(
        "\tmove $a0, %0\n"
        "\tmove $a1, %1\n"
        "\tnop\n"
        "\tsyscall\n"
        "\tnop\n"
        : : "r"(number), "r"(arg)
    );
    va_end(arg);

    register int v0 asm("v0");
    return v0;
}

extern struct kernel_syscall syscall_table[];
extern uint8_t __syscalls_start;
extern uint8_t __syscalls_end;

struct user_regs *syscall_exception_handler(struct user_regs *current_regs) {
    const size_t n_syscalls = ((&__syscalls_end - &__syscalls_start) / sizeof(struct kernel_syscall));
    int ret = -1;
    struct user_regs *ctx_switch_in_regs = current_regs;

    for (size_t i = 0; i < n_syscalls; i++) {
        struct kernel_syscall* current = &syscall_table[i];
        if (current_regs->a0 == current->number) {
            ret = scheduler_syscall_handler(&ctx_switch_in_regs, current, (va_list) current_regs->a1);
            break;
        }
    }

    current_regs->v0 = (uint32_t) ret;
    current_regs->epc += 4;
    return ctx_switch_in_regs;
}
