#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <interrupts.h>
#include <mips.h>
//#include <tlb.h>
//#include <pmap.h>
#include <platform/panic.h>
#include <platform/kprintf.h>

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
//  write_c0_intctl(read_c0_intctl() | 1 << 5);

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
  [EXC_MOD]  = "TLB modification exception",
  [EXC_TLBL] = "TLB exception (load or instruction fetch)",
  [EXC_TLBS] = "TLB exception (store)",
  [EXC_ADEL] = "Address error exception (load or instruction fetch)",
  [EXC_ADES] = "Address error exception (store)",
  [EXC_IBE]  = "Bus error exception (instruction fetch)",
  [EXC_DBE]  = "Bus error exception (data reference: load or store)",
  [EXC_BP]   = "Breakpoint exception",
  [EXC_RI]   = "Reserved instruction exception",
  [EXC_CPU]  = "Coprocessor Unusable exception",
  [EXC_OVF]  = "Arithmetic Overflow exception",
  [EXC_TRAP] = "Trap exception",
  [EXC_FPE]  = "Floating point exception",
  [EXC_WATCH] = "Reference to watchpoint address",
  [EXC_MCHECK] = "Machine checkcore",
};

#define PDE_ID_FROM_PTE_ADDR(x) (((x) & 0x003ff000) >> 12)

void tlb_exception_handler()
{
  int code = (mips32_get_c0(C0_CAUSE) & CR_X_MASK) >> CR_X_SHIFT;
  uint32_t vaddr = mips32_get_c0(C0_BADVADDR);
  kprintf("[tlb] %s at $%08lx!\n", exceptions[code], mips32_get_c0(C0_EPC));
  kprintf("[tlb] Caused by reference to $%08lx!\n", vaddr);

  panic("TLB exception not implemented!");
#if 0
  assert(PTE_BASE <= vaddr && vaddr < PTE_BASE+PTE_SIZE);
  /* If the fault was in virtual pt range it means it's time to refill */
  kprintf("[tlb] pde_refill\n");
  uint32_t id = PDE_ID_FROM_PTE_ADDR(vaddr);
  tlbhi_t entryhi = mips32_get_c0(C0_ENTRYHI);

  pmap_t *active_pmap = get_active_pmap();
  if(!(active_pmap->pde[id] & V_MASK))
    panic("Trying to access unmapped memory region.\
            You probably deferred NULL or there was stack overflow. ");

  id &= ~1;
  pte_t entrylo0 = active_pmap->pde[id];
  pte_t entrylo1 = active_pmap->pde[id+1];
  tlb_overwrite_random(entryhi, entrylo0, entrylo1);
#endif
}

void kernel_oops() {
  int code = (mips32_get_c0(C0_CAUSE) & CR_X_MASK) >> CR_X_SHIFT;

  kprintf("[oops] %s at $%08x!\n", exceptions[code], mips32_get_c0(C0_EPC));
  if (code == EXC_ADEL || code == EXC_ADES) {
    kprintf("[oops] Caused by reference to $%08x!\n", mips32_get_c0(C0_BADVADDR));
  }

  panic("Unhandled exception");
}

/* 
 * Following is general exception table. General exception handler has very
 * little space to use. So it loads address of handler from here. All functions
 * being jumped to, should have ((interrupt)) attribute, unless some exception
 * is unhandled, then these functions should panic the kernel.  For exact
 * meanings of exception handlers numbers please check 5.23 Table of MIPS32
 * 4KEc User's Manual. 
 */

void *general_exception_table[32] = {
  [EXC_MOD]  = tlb_exception_handler,
  [EXC_TLBL] = tlb_exception_handler,
  [EXC_TLBS] = tlb_exception_handler,
};

