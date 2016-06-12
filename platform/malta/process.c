//
// Created by gz on 6/12/16.
//

#include <platform/process.h>
#include <platform/malta/interrupts.h>
#include <stdio.h>

struct user_regs* platform_initialize_process_stack(uint8_t* stackPointer, size_t stackSize,
                                                    struct process_entry_info* info)
{
    // stack is in a descending order
    // and should be initialized with struct user_regs in order to start
    if (stackSize < sizeof(struct user_regs)) {
        return NULL;
    }

    struct user_regs* context = (struct user_regs*)(stackPointer + stackSize - sizeof(struct user_regs));
    context->epc = (uint32_t)info->entryPoint;
    context->a0 = (uint32_t)info->argument;

    printf("in platform_initialize_process_stack: epc %p a0 %p\n", info->entryPoint, info->argument);

    return context;
}