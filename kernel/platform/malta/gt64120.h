#ifndef __GT64120_H__
#define __GT64120_H__

#include <platform/malta/malta.h>
#include <stdint.h>

#define GT_R(x) *((volatile uint32_t *) \
    (MIPS_PHYS_TO_KSEG1(MALTA_CORECTRL_BASE + (x))))

#define GT_DEF_R(x) *((volatile uint32_t *) \
    (MIPS_PHYS_TO_KSEG1(GT_DEF_BASE + (x))))

struct gt64120_regs {
    uint8_t	 unused_000[0xc18];
    uint32_t intrcause;
    uint8_t  unused_c1c[0x0dc];
    uint32_t pci0_cfgaddr;
    uint32_t pci0_cfgdata;
};

/* Table 188: PCI_0 Configuration Address */
#define GT_PCI0_CFG_ADDR 0xcf8
/* Table 189: PCI_0 Configuration Data */
#define GT_PCI0_CFG_DATA 0xcfc

/* Table 194: Interrupts Register Cause Register */
#define GT_INTR_CAUSE  0xc18
#define GTIC_INTSUM    0x00000001
#define GTIC_PCIINTSUM 0x80000000
/* Table 198: CPU Interrupt Mask Register */
#define GT_INTR_MASK   0xc1c
#define GTIM_MEMOUT    0x00000002
#define GTIM_DMAOUT    0x00000004
#define GTIM_CPUOUT    0x00000008
#define GTIM_DMA0COMP  0x00000010
#define GTIM_DMA1COMP  0x00000020
#define GTIM_DMA2COMP  0x00000040
#define GTIM_DMA3COMP  0x00000080
#define GTIM_T0EXP     0x00000100
#define GTIM_T1EXP     0x00000200
#define GTIM_T2EXP     0x00000400
#define GTIM_T3EXP     0x00000800
#define GTIM_MASRDERR0 0x00001000
#define GTIM_SLVWRERR0 0x00002000
#define GTIM_MASWRERR0 0x00004000
#define GTIM_SLVRDERR0 0x00008000
#define GTIM_ADDRERR0  0x00010000
#define GTIM_MEMERR    0x00020000
#define GTIM_MASABORT0 0x00040000
#define GTIM_TARABORT0 0x00080000
#define GTIM_RETRYCNT0 0x00100000
#define GTIM_PMCINT_0  0x00200000
#define GTIM_CPUINT    0x0c300000
#define GTIM_PCINT     0xc3000000
#define GTIM_CPUINTSUM 0x40000000
/* Table 200: PCI_0 Interrupt Cause Mask */
#define GT_PCI0_INTR_MASK 0xc24

#define GT_DEF_BASE         0x14000000
#define GT_PCI0IOLD_OFS     0x048
#define GT_PCI0IOHD_OFS     0x050
#define GT_PCI0M0LD_OFS     0x058
#define GT_PCI0M0HD_OFS     0x060
#define GT_ISD_OFS		    0x068
#define GT_PCI0M1LD_OFS     0x080
#define GT_PCI0M1HD_OFS     0x088
#define GT_PCI1IOLD_OFS     0x090
#define GT_PCI1IOHD_OFS     0x098
#define GT_PCI1M0LD_OFS     0x0a0
#define GT_PCI1M0HD_OFS     0x0a8
#define GT_PCI1M1LD_OFS     0x0b0
#define GT_PCI1M1HD_OFS     0x0b8
#define GT_PCI1M1LD_OFS     0x0b0
#define GT_PCI1M1HD_OFS     0x0b8


#include <platform/malta/gt64120_full.h>
#include <platform/malta/pci_registers.h>

#endif
