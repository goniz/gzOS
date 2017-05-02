#include <stdint.h>
#include <mips.h>
#include <platform/interrupts.h>
#include <platform/malta/interrupts.h>
#include <platform/panic.h>
#include <platform/kprintf.h>
#include <assert.h>
#include <lib/mm/pmap.h>
#include <sys/cdefs.h>

extern const char _ebase[];
extern int is_in_irq;
extern int uart_puts(const char *s);
struct user_regs *syscall_exception_handler(struct user_regs *current_regs);

void interrupts_enable_all(void) {
    asm volatile("ei");
}

unsigned int interrupts_enable_save(void) {
    unsigned int status = mips32_get_c0(C0_STATUS);
    interrupts_enable_all();
    return status;
}

unsigned int interrupts_disable(void) {
    return _mips_intdisable();
}

void interrupts_enable(unsigned int mask) {
    mips32_setsr(mask);
}

void platform_enable_hw_irq(int irq) {
    assert(irq >= 0 && irq <= 7);

    unsigned int irqMask = interrupts_disable();
    irqMask |= (1 << (SR_IMASK_SHIFT + irq));
    interrupts_enable(irqMask);
}

void platform_disable_hw_irq(int irq) {
    assert(irq >= 0 && irq <= 7);

    unsigned int irqMask = interrupts_disable();
    irqMask &= ~(1 << (SR_IMASK_SHIFT + irq));
    interrupts_enable(irqMask);
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

    /*
     * Mask out software and hardware interrupts. 
     * You should enable them one by one in driver initialization code.
     */
    // IPL bits are bits 10-15 in SR (part of IM0-7)
    mips32_bc_c0(C0_STATUS, 0xfc00);

    interrupts_enable_all();
}

const char *exceptions[32] = {
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
        [EXC_TLBRI]     = "TLB read inhibit",
        [EXC_TLBXI]	    = "TLB execute inhibit",
        [EXC_WATCH] = "Reference to watchpoint address",
        [EXC_MCHECK] = "Machine checkcore",
};

void kernel_oops() {
    uint32_t code = (mips32_get_c0(C0_CAUSE) & CR_X_MASK) >> CR_X_SHIFT;
    uint32_t ip = (mips32_get_c0(C0_CAUSE) & CR_IP_MASK) >> CR_IP_SHIFT;
    uint32_t im = (mips32_get_c0(C0_STATUS) & SR_IMASK) >> SR_IMASK_SHIFT;

    kprintf("IP: %08x\n", ip);
    kprintf("IM: %08x\n", im);

    kprintf("[oops] %s (IP&IM: %08x) at $%08x!\n", exceptions[code], ip & im, mips32_get_c0(C0_EPC));
    if (code == EXC_ADEL || code == EXC_ADES) {
        kprintf("[oops] Caused by reference to $%08x!\n", mips32_get_c0(C0_BADVADDR));
    }

    panic("Unhandled exception");
}

void print_user_regs(struct user_regs *regs) {
    kprintf("$a0 %08x $a1 %08x $a2 %08x $a3 %08x\n", regs->a0, regs->a1, regs->a2, regs->a3);
    kprintf("$t0 %08x $t1 %08x $t2 %08x $t3 %08x $t4 %08x $t5 %08x $t6 %08x $t7 %08x $t8 %08x $t9 %08x\n",
            regs->t0, regs->t1, regs->t2, regs->t3, regs->t4, regs->t5, regs->t6, regs->t7, regs->t8, regs->t9);
    kprintf("$s0 %08x $s1 %08x $s2 %08x $s3 %08x $s4 %08x $s5 %08x $s6 %08x $s7 %08x\n",
            regs->s0, regs->s1, regs->s2, regs->s3, regs->s4, regs->s5, regs->s6, regs->s7);
    kprintf("$fp %08x $at %08x $gp %08x $hi %08x $lo %08x $ra %08x $epc %08x\n", regs->fp, regs->at, regs->gp,
            regs->hi,
            regs->lo, regs->ra, regs->epc);
}

/* 
 * Following is general exception table. General exception handler has very
 * little space to use. So it loads address of handler from here. All functions
 * being jumped to, should have ((interrupt)) attribute, unless some exception
 * is unhandled, then these functions should panic the kernel.    For exact
 * meanings of exception handlers numbers please check 5.23 Table of MIPS32
 * 4KEc User's Manual. 
 */

__used
void *general_exception_table[32] = {
        [EXC_MOD]   = tlb_exception_handler,
        [EXC_TLBL]  = tlb_exception_handler,
        [EXC_TLBS]  = tlb_exception_handler,
        [EXC_SYS]   = syscall_exception_handler,
};

#define IRQ_STUB(num) \
    __attribute__((weak)) \
    struct user_regs* mips_hw_irq ##num (struct user_regs* regs) { \
        kernel_oops(); \
        return regs; \
    }

IRQ_STUB(0);
IRQ_STUB(1);
IRQ_STUB(2);
IRQ_STUB(3);
IRQ_STUB(4);
IRQ_STUB(5);
IRQ_STUB(6);
IRQ_STUB(7);
