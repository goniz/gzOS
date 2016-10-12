//
// Created by gz on 6/11/16.
//

#ifndef GZOS_PANIC_H
#define GZOS_PANIC_H

#include <platform/interrupts.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((noreturn))
void panic(const char* fmt, ...);

void print_user_regs(struct user_regs* regs);

#ifdef __cplusplus
}
#endif

#endif //GZOS_PANIC_H
