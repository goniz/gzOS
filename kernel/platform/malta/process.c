#include <mips/m32c0.h>
#include <platform/process.h>
#include <platform/malta/interrupts.h>
#include <string.h>
#include <platform/kprintf.h>


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

void platform_set_active_kernel_stack(void* stack_pointer) {
//    kprintf("setting C0_USERLOCAL to %p\n", stack_pointer);
    mips32_set_c0(C0_USERLOCAL, stack_pointer);
}