#ifndef GZOS_PLAT_PROCESS_H
#define GZOS_PLAT_PROCESS_H

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <platform/interrupts.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct user_regs* stack_pointer;
} platform_thread_cb;

int platform_is_in_userspace_range(uintptr_t start, uintptr_t end);

void platform_set_active_thread(platform_thread_cb* cb);

struct user_regs* platform_initialize_stack(void* stack, size_t stack_size,
                                            void* user_stack,
                                            void* entryPoint, void* argument,
                                            void* return_address,
                                            int is_kernel_thread);

#ifdef __cplusplus
}
#endif

#else
    #define THREAD_STACK    0
#endif // __ASSEMBLER__
#endif //GZOS_PLAT_PROCESS_H
