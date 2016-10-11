#include <mips/m32c0.h>
#include <platform/process.h>
#include <platform/malta/interrupts.h>
#include <platform/panic.h>
#include <string.h>

__attribute__((noreturn))
static void hang(void)
{
	panic("Returned from main somehow.. hanging...\n");
}

extern uint32_t _gp;
struct user_regs* platform_initialize_stack(void* stack, size_t stack_size, void* entryPoint, void* argument, void* return_address)
{
    struct user_regs* context = (struct user_regs*)(stack + stack_size - sizeof(struct user_regs) - sizeof(int));
    memset(context, 0, sizeof(*context));
    context->epc = (uint32_t)entryPoint;
    context->a0 = (uint32_t)argument;
    context->gp = (uint32_t) &_gp;
    context->ra = (uint32_t) (return_address ? return_address : hang);
//    context->status =

    return context;
}
