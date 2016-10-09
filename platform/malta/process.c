#include <platform/process.h>
#include <platform/malta/interrupts.h>
#include <stdlib.h>
#include <lib/mm/vm.h>
#include <lib/mm/physmem.h>
#include <lib/mm/pmap.h>
#include <string.h>
#include <platform/kprintf.h>
#include <platform/panic.h>

struct platform_process_ctx {
    pid_t pid;
    struct _reent reent;
};

struct platform_process_ctx* platform_initialize_process_ctx(pid_t pid)
{
    struct platform_process_ctx* pctx = malloc(sizeof(struct platform_process_ctx));
    if (NULL == pctx) {
        return NULL;
    }

    pctx->pid = pid;
    _REENT_INIT_PTR((&pctx->reent));
    return pctx;
}

void platform_free_process_ctx(struct platform_process_ctx* pctx)
{
    if (NULL == pctx) {
        return;
    }

    pctx->pid = 0;
    _reclaim_reent(&pctx->reent);
    free(pctx);
}

__attribute__((noreturn))
static void hang(void)
{
	panic("Returned from main somehow.. hanging...\n");
}

extern uint32_t _gp;
struct user_regs* platform_initialize_stack(void* stack, size_t stack_size, void* entryPoint, void* argument, void* return_address)
{
    struct user_regs* context = (struct user_regs*)(stack + stack_size - sizeof(struct user_regs) - sizeof(int));
    context->epc = (uint32_t)entryPoint;
    context->a0 = (uint32_t)argument;
    context->gp = (uint32_t) &_gp;
    context->ra = (uint32_t) (return_address ? return_address : hang);

    return context;
}

void platform_set_active_process_ctx(struct platform_process_ctx* pctx)
{
    _REENT = &pctx->reent;
}

void platform_leave_process_ctx(void)
{
    // TODO: save original reent and restore it here..
//    _impure_ptr = NULL;
}
