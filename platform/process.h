//
// Created by gz on 6/12/16.
//

#ifndef GZOS_PLAT_PROCESS_H
#define GZOS_PLAT_PROCESS_H

#include <stdint.h>
#include <stddef.h>
#include <platform/interrupts.h>

#ifdef __cplusplus
extern "C" {
#endif

struct process_entry_info {
    void (*entryPoint)(void*);
    void* argument;
};

struct user_regs* platform_initialize_process_stack(uint8_t* stackPointer, size_t stackSize,
                                                    struct process_entry_info* info);

#ifdef __cplusplus
}
#endif
#endif //GZOS_PLAT_PROCESS_H
