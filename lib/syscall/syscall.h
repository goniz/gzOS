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
 * DEFINE_SYSCALL(SYSCALL_NR_WRITE, write)
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

struct kernel_syscall {
    uint32_t number;
    sys_handler_t handler;
};

#define DEFINE_SYSCALL(number, name) \
            extern "C" \
            __attribute__((used)) int sys_ ##name(struct user_regs** regs, va_list args); \
            __attribute__((section(".syscalls"),used)) \
            static const struct kernel_syscall __sys_ ##name## _ks = { (number), (sys_handler_t) sys_ ##name }; \
            extern "C" \
            __attribute__((used)) int sys_ ##name(struct user_regs** regs, va_list args)

#define SYSCALL_ARG(type, name) \
            type name = va_arg(args, type)

#define DECLARE_SYSCALL_NR(name) SYS_NR_ ##name

/* basic syscall invocation call */
int syscall(int number, ...);

enum system_call_numbers
{
    DECLARE_SYSCALL_NR(CREATE_PREEMPTIVE_PROC),
    DECLARE_SYSCALL_NR(CREATE_RESPONSIVE_PROC),
    DECLARE_SYSCALL_NR(GET_PID),
    DECLARE_SYSCALL_NR(YIELD),
    DECLARE_SYSCALL_NR(SIGNAL),
    DECLARE_SYSCALL_NR(PS),
    DECLARE_SYSCALL_NR(SOCKET),
    DECLARE_SYSCALL_NR(BIND),
};


#ifdef __cplusplus
};
#endif
#endif //GZOS_SYSCALL_H
