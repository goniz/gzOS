//
// Created by gz on 6/25/16.
//

#ifndef GZOS_SYSCALL_H
#define GZOS_SYSCALL_H

#include <platform/interrupts.h>
#include <stdarg.h>
#include <stdint.h>

/*
 * typical system call definition looks like this:
 * DEFINE_SYSCALL(WRITE, write, SYS_IRQ_ENABLED/SYS_IRQ_DISABLED)
 * {
 *      SYSCALL_ARG(int, fd);
 *      SYSCALL_ARG(const char*, buffer);
 *      SYSCALL_ARG(size_t, size);
 *      // TODO: implement
 *      return -1;
 * }
 *
 * typical system call invocation looks like this:
 * int ret = syscall(SYSCALL_NR_WRITE, fd, buffer, sizeof(buffer));
 * */

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*sys_handler_t)(struct user_regs** regs, va_list args);
typedef enum {
    SYS_IRQ_ENABLED,
    SYS_IRQ_DISABLED
} syscall_irq_t;

struct kernel_syscall {
    uint32_t number;
    sys_handler_t handler;
    syscall_irq_t irq;
};

#define DEFINE_SYSCALL(number, name, irq) \
            extern "C" \
            __attribute__((used)) int sys_ ##name(struct user_regs** regs, va_list args); \
            __attribute__((section(".syscalls"),used)) \
            static const struct kernel_syscall __sys_ ##name## _ks = { \
                ( SYS_NR_ ##number ), \
                (sys_handler_t) sys_ ##name , \
                (syscall_irq_t) (irq) \
            }; \
            extern "C" \
            __attribute__((used)) int sys_ ##name(struct user_regs** regs, va_list args)

#define SYSCALL_ARG(type, name) \
            type name = va_arg(args, type)

#define DECLARE_SYSCALL_NR(name) SYS_NR_ ##name

/* basic syscall invocation call */
int syscall(int number, ...);

enum system_call_numbers
{
    DECLARE_SYSCALL_NR(CREATE_PROCESS),
    DECLARE_SYSCALL_NR(CREATE_THREAD),
    DECLARE_SYSCALL_NR(GET_PID),
    DECLARE_SYSCALL_NR(GET_TID),
    DECLARE_SYSCALL_NR(IS_THREAD_RESPONSIVE),
    DECLARE_SYSCALL_NR(SET_THREAD_RESPONSIVE),
    DECLARE_SYSCALL_NR(SCHEDULE),
    DECLARE_SYSCALL_NR(YIELD),
    DECLARE_SYSCALL_NR(SIGNAL),
    DECLARE_SYSCALL_NR(SLEEP),
    DECLARE_SYSCALL_NR(PS),
    DECLARE_SYSCALL_NR(OPEN),
    DECLARE_SYSCALL_NR(CLOSE),
    DECLARE_SYSCALL_NR(READ),
    DECLARE_SYSCALL_NR(WRITE),
    DECLARE_SYSCALL_NR(SOCKET),
    DECLARE_SYSCALL_NR(BIND),
    DECLARE_SYSCALL_NR(CONNECT),
    DECLARE_SYSCALL_NR(RECVFROM),
    DECLARE_SYSCALL_NR(SENDTO)
};


#ifdef __cplusplus
};
#endif
#endif //GZOS_SYSCALL_H