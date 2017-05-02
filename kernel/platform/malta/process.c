#include <mips/m32c0.h>
#include <platform/process.h>
#include <platform/malta/interrupts.h>
#include <string.h>
#include <platform/kprintf.h>
#include <assert.h>
#include "mips.h"


extern uint32_t _gp;
struct user_regs* platform_initialize_stack(void* stack, size_t stack_size,
                                            void* user_stack,
                                            void* entryPoint, void* argument,
                                            void* return_address,
                                            int is_kernel_thread)
{
    struct user_regs* context = (struct user_regs*)(stack + stack_size - sizeof(struct user_regs) - sizeof(int));

    memset(context, 0, sizeof(*context));
    context->epc = (uint32_t)entryPoint;
    context->a0 = (uint32_t)argument;
    context->gp = (uint32_t) &_gp; // TODO: use user gp?
    context->ra = (uint32_t) return_address;
    context->sp = (uint32_t) (user_stack ? user_stack : stack + stack_size);
    context->status = mips32_get_c0(C0_STATUS) | SR_IE;
    context->status &= ~(SR_EXL | SR_ERL);

    if (is_kernel_thread) {
        context->status |= SR_KSU_KERN;
    } else {
        context->status |= SR_KSU_USER;
    }

    return context;
}

struct user_regs* platform_copy_stack(void* dstStack, size_t dstSize, const struct user_regs* srcStack)
{
    struct user_regs* context = (struct user_regs*)(dstStack + dstSize - sizeof(struct user_regs) - sizeof(int));

    memcpy(context, srcStack, sizeof(struct user_regs));

    return context;
}

extern platform_thread_cb* _thread_cb_pointer;
void platform_set_active_thread(platform_thread_cb* cb) {
    assert(NULL != cb);
    _thread_cb_pointer = cb;
}

void platform_thread_set_return_value(platform_thread_cb* cb, uintptr_t return_value) {
    assert(NULL != cb);
    assert(NULL != cb->stack_pointer);

    cb->stack_pointer->v0 = return_value;
}

void platform_thread_advance_pc(platform_thread_cb* cb, uintptr_t opcodes) {
    assert(NULL != cb);
    assert(NULL != cb->stack_pointer);

    cb->stack_pointer->epc += (opcodes * sizeof(uint32_t));
}

int platform_is_in_userspace_range(uintptr_t start, uintptr_t end) {
    if (start >= 0 && end <= MIPS_KSEG0_START) {
        return 1;
    }

    return 0;
}
