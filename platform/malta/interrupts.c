#include <stdint.h>
#include <mips.h>
#include <platform/interrupts.h>
#include <platform/malta/interrupts.h>
//#include <tlb.h>
//#include <pmap.h>
#include <platform/panic.h>
#include <platform/kprintf.h>
#include <assert.h>
#include <lib/syscall/syscall.h>
#include "tlb.h"
#include "pmap.h"

extern const char _ebase[];
extern int uart_puts(const char* s);

void interrupts_enable_all(void)
{
	asm volatile("ei");
}

unsigned int interrupts_disable(void)
{
	register unsigned int isrMask = 0;
	asm volatile("di %0" : "=r"(isrMask));
	return isrMask;
}

void interrupts_enable(unsigned int mask)
{
	asm volatile("mtc0 %0, $12" : : "r"(mask));
}

void interrupts_init() {
    /*
     * Enable Vectored Interrupt Mode as described in „MIPS32® 24KETM Processor
     * Core Family Software User’s Manual”, chapter 6.3.1.2.
     */

    /* The location of exception vectors is set to EBase. */
    mips32_set_c0(C0_EBASE, _ebase);
    mips32_bc_c0(C0_STATUS, SR_BEV);
    /* Use the special interrupt vector at EBase + 0x200. */
    mips32_bs_c0(C0_CAUSE, CR_IV);
    /* Set vector spacing for 0x20. */
    mips32_set_c0(C0_INTCTL, INTCTL_VS_32);
//    write_c0_intctl(read_c0_intctl() | 1 << 5);

    /*
     * Mask out software and hardware interrupts. 
     * You should enable them one by one in driver initialization code.
     */
    // IPL bits are bits 10-15 in SR (part of IM0-7)
    mips32_bc_c0(C0_STATUS, 0xfc00);

    interrupts_enable_all();
}

__attribute__((used))
static const char *exceptions[32] = {
    [EXC_INTR] = "Interrupt",
    [EXC_MOD]    = "TLB modification exception",
    [EXC_TLBL] = "TLB exception (load or instruction fetch)",
    [EXC_TLBS] = "TLB exception (store)",
    [EXC_ADEL] = "Address error exception (load or instruction fetch)",
    [EXC_ADES] = "Address error exception (store)",
    [EXC_IBE]    = "Bus error exception (instruction fetch)",
    [EXC_DBE]    = "Bus error exception (data reference: load or store)",
    [EXC_BP]     = "Breakpoint exception",
    [EXC_RI]     = "Reserved instruction exception",
    [EXC_CPU]    = "Coprocessor Unusable exception",
    [EXC_OVF]    = "Arithmetic Overflow exception",
    [EXC_TRAP] = "Trap exception",
    [EXC_FPE]    = "Floating point exception",
    [EXC_WATCH] = "Reference to watchpoint address",
    [EXC_MCHECK] = "Machine checkcore",
};

#define PDE_ID_FROM_PTE_ADDR(x) (((x) & 0x003ff000) >> 12)

struct user_regs* tlb_exception_handler(struct user_regs* regs)
{
    uint32_t code = (mips32_get_c0(C0_CAUSE) & CR_X_MASK) >> CR_X_SHIFT;
    uint32_t vaddr = mips32_get_c0(C0_BADVADDR);
    uint32_t epc = mips32_get_c0(C0_EPC);

    kprintf("[tlb] %s at $%08x!\n", exceptions[code], epc);
    kprintf("[tlb] Caused by reference to $%08x!\n", vaddr);

//    assert(PTE_BASE <= vaddr && vaddr < PTE_BASE+PTE_SIZE);
    /* If the fault was in virtual pt range it means it's time to refill */
    kprintf("[tlb] pde_refill\n");
    uint32_t id = PDE_ID_FROM_PTE_ADDR(vaddr);
    kprintf("[tlb] pde id %ld\n", id);
    tlbhi_t entryhi = mips32_get_c0(C0_ENTRYHI);
    kprintf("[tlb] entryhi %08x\n", entryhi);

    pmap_t *active_pmap = get_active_pmap();
    kprintf("[tlb] active_pmap %p\n", active_pmap);
    if ((void*)0 == active_pmap) {
        kprintf("[tlb] no active pmap\n");
        panic("NO ACTIVE PMAP..");
    }

    if(!(active_pmap->pde[id] & V_MASK)) {
        panic("Trying to access unmapped memory region.\
            You probably deferred NULL or there was stack overflow. ");
    }

    id &= ~1;
    pte_t entrylo0 = active_pmap->pde[id];
    pte_t entrylo1 = active_pmap->pde[id+1];
    tlb_overwrite_random(entryhi, entrylo0, entrylo1);
    return regs;
}

extern struct kernel_syscall syscall_table[];
extern uint8_t __syscalls_start;
extern uint8_t __syscalls_end;
struct user_regs* syscall_exception_handler(struct user_regs* current_regs)
{
    const size_t n_syscalls = ((&__syscalls_end - &__syscalls_start) / sizeof(struct kernel_syscall));
    int ret = -1;
    struct user_regs* ctx_switch_in_regs = current_regs;

    for (size_t i = 0; i < n_syscalls; i++) {
        struct kernel_syscall* currrent = &syscall_table[i];
        if (current_regs->a0 == currrent->number) {
            kprintf("%s: calling syscall %d handler %p\n", __FUNCTION__, currrent->number, currrent->handler);
            ret = currrent->handler(&ctx_switch_in_regs, (va_list)current_regs->a1);
            break;
        }
    }

    current_regs->v0 = (uint32_t) ret;
    current_regs->epc += 4;
    return ctx_switch_in_regs;
}

void kernel_oops() {
    uint32_t code = (mips32_get_c0(C0_CAUSE) & CR_X_MASK) >> CR_X_SHIFT;

    kprintf("[oops] %s at $%08x!\n", exceptions[code], mips32_get_c0(C0_EPC));
    if (code == EXC_ADEL || code == EXC_ADES) {
        kprintf("[oops] Caused by reference to $%08x!\n", mips32_get_c0(C0_BADVADDR));
    }

    panic("Unhandled exception");
}

void print_user_regs(struct user_regs* regs)
{
    kprintf("$a0 %08x $a1 %08x $a2 %08x $a3 %08x\n", regs->a0, regs->a1, regs->a2, regs->a3);
    kprintf("$t0 %08x $t1 %08x $t2 %08x $t3 %08x $t4 %08x $t5 %08x $t6 %08x $t7 %08x $t8 %08x $t9 %08x\n",
            regs->t0, regs->t1, regs->t2, regs->t3, regs->t4, regs->t5, regs->t6, regs->t7, regs->t8, regs->t9);
    kprintf("$s0 %08x $s1 %08x $s2 %08x $s3 %08x $s4 %08x $s5 %08x $s6 %08x $s7 %08x\n",
            regs->s0, regs->s1, regs->s2, regs->s3, regs->s4, regs->s5, regs->s6, regs->s7);
    kprintf("$fp %08x $at %08x $gp %08x $hi %08x $lo %08x $ra %08x $epc %08x\n", regs->fp, regs->at, regs->gp, regs->hi, regs->lo, regs->ra, regs->epc);
}

/* 
 * Following is general exception table. General exception handler has very
 * little space to use. So it loads address of handler from here. All functions
 * being jumped to, should have ((interrupt)) attribute, unless some exception
 * is unhandled, then these functions should panic the kernel.    For exact
 * meanings of exception handlers numbers please check 5.23 Table of MIPS32
 * 4KEc User's Manual. 
 */

void *general_exception_table[32] = {
    [EXC_MOD]    = tlb_exception_handler,
    [EXC_TLBL] = tlb_exception_handler,
    [EXC_TLBS] = tlb_exception_handler,
    [EXC_SYS] = syscall_exception_handler,
};

