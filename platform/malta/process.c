//
// Created by gz on 6/12/16.
//

#include <platform/process.h>
#include <platform/malta/interrupts.h>
#include <stdio.h>
#include <platform/malta/physmem.h>
#include <lib/primitives/align.h>
#include <stdlib.h>
#include <platform/kprintf.h>
#include "pmap.h"

#define VIRT_STACK_BASE 0x5000

struct platform_process_ctx {
    pid_t pid;
    size_t stack_size;
    struct pmap pmap;
    vm_page_t* stack_base;
};

struct platform_process_ctx* platform_initialize_process_ctx(pid_t pid, size_t stackSize)
{
    struct platform_process_ctx* pctx = malloc(sizeof(struct platform_process_ctx));
    if (NULL == pctx) {
        return NULL;
    }

    pctx->pid = pid;
    pctx->stack_size = stackSize;
    pmap_init(&pctx->pmap);
    pctx->pmap.asid = 10;

    pctx->stack_base = pm_alloc_bytes(stackSize);
    if (NULL == pctx->stack_base) {
        platform_free_process_ctx(pctx);
        return NULL;
    }

    // TODO: enable virt stack mapping after mapping swap on ctx switch is implemented..
//    kprintf("Allocated stack_base: virt %08x phy %08x real virt %08x\n", pctx->stack_base->virt_addr, pctx->stack_base->phys_addr, VIRT_STACK_BASE);

//    pmap_t* old_pmap = get_active_pmap();
//    set_active_pmap(&pctx->pmap);
//    pmap_map(&pctx->pmap, VIRT_STACK_BASE, pctx->stack_base->phys_addr, 1 << pctx->stack_base->order, PMAP_VALID | PMAP_DIRTY);
//    set_active_pmap(old_pmap);

    return pctx;
}

void platform_free_process_ctx(struct platform_process_ctx* pctx)
{
    if (NULL == pctx) {
        return;
    }

    pctx->pid = 0;
    pctx->stack_size = 0;
    pmap_delete(&pctx->pmap);

    if (NULL != pctx->stack_base) {
        pm_free(pctx->stack_base);
    }

    free(pctx);
}

extern uint32_t _gp;
struct user_regs* platform_initialize_process_stack(struct platform_process_ctx* pctx,
                                                    struct process_entry_info* info)
{
    kprintf("platform_initialize_process_stack: epc %p a0 %p stack base %p stack size %d\n",
            info->entryPoint, info->argument,
            pctx->stack_base, pctx->stack_size
    );

    // stack is in a descending order
    // and should be initialized with struct user_regs in order to start

    // save the old pmap and set the current one
    // then initialize it with the virt stack base
//    pmap_t* old_pmap = get_active_pmap();
//    set_active_pmap(&pctx->pmap);
//    struct user_regs* context = (struct user_regs*)(VIRT_STACK_BASE + pctx->stack_size - sizeof(struct user_regs) - sizeof(int));
    struct user_regs* context = (struct user_regs*)(pctx->stack_base->vaddr + pctx->stack_size - sizeof(struct user_regs) - sizeof(int));
    context->epc = (uint32_t)info->entryPoint;
    context->a0 = (uint32_t)info->argument;
    context->gp = (uint32_t) &_gp;
//    set_active_pmap(old_pmap);

    return context;
}

void platform_set_active_process_ctx(struct platform_process_ctx* pctx)
{
    set_active_pmap(&pctx->pmap);
}

void platform_leave_process_ctx(void)
{
    set_active_pmap(NULL);
}
