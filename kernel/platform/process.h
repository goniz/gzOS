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

struct user_regs* platform_initialize_stack(void* stack,
                                            size_t stack_size,
                                            void* entryPoint,
                                            void* argument,
                                            void* return_address);

#ifdef __cplusplus
}
#endif
#endif //GZOS_PLAT_PROCESS_H
