//
// Created by gz on 6/12/16.
//

#ifndef GZOS_PLAT_PROCESS_H
#define GZOS_PLAT_PROCESS_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <platform/interrupts.h>

#ifdef __cplusplus
extern "C" {
#endif

struct process_entry_info {
    void (*entryPoint)(void*);
    void* argument;
};

struct platform_process_ctx;

struct user_regs* platform_initialize_stack(void* stack,
                                            size_t stack_size,
                                            void* entryPoint,
                                            void* argument,
                                            void* return_address);

struct platform_process_ctx* platform_initialize_process_ctx(pid_t pid);
void platform_free_process_ctx(struct platform_process_ctx* pctx);

void platform_set_active_process_ctx(struct platform_process_ctx* pctx);
void platform_leave_process_ctx(void);

#ifdef __cplusplus
}
#endif
#endif //GZOS_PLAT_PROCESS_H
