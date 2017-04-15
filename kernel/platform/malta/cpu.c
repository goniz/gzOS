#include <stdint.h>
#include <platform/kprintf.h>
#include <platform/malta/mips.h>
#include <platform/malta/malta.h>
#include <assert.h>
#include <platform/cpu.h>
#include "gt64120.h"

typedef struct cpuinfo {
        int tlb_entries;
        int ic_size;
        int ic_linesize;
        int ic_nways;
        int ic_nsets;
        int dc_size;
        int dc_linesize;
        int dc_nways;
        int dc_nsets;
} cpuinfo_t;

static cpuinfo_t cpuinfo;

/* 
 * Read configuration register values, interpret and save them into the cpuinfo
 * structure for later use.
 */
int platform_read_cpu_config(void)
{
    uint32_t config0 = mips32_getconfig0();
    uint32_t cfg0_mt = config0 & CFG0_MT_MASK;
    char *cfg0_mt_str;

    if (cfg0_mt == CFG0_MT_TLB)
        cfg0_mt_str = "Standard TLB";
    else if (cfg0_mt == CFG0_MT_BAT)
        cfg0_mt_str = "BAT";
    else if (cfg0_mt == CFG0_MT_FIXED)
        cfg0_mt_str = "Fixed mapping";
    else if (cfg0_mt == CFG0_MT_DUAL)
        cfg0_mt_str = "Dual VTLB and FTLB";
    else
        cfg0_mt_str = "No MMU";

    kprintf("MMU Type: %s\n", cfg0_mt_str);

    /* CFG1 implemented? */
    if ((config0 & CFG0_M) == 0)
        return 0;

    uint32_t config1 = mips32_getconfig1();

    /* FTLB or/and VTLB sizes */
    cpuinfo.tlb_entries = _mips32r2_ext(config1, CFG1_MMUS_SHIFT, CFG1_MMUS_BITS) + 1;

    /* Instruction cache size and organization. */
    cpuinfo.ic_linesize = (config1 & CFG1_IL_MASK) ? 32 : 0;
    cpuinfo.ic_nways = _mips32r2_ext(config1, CFG1_IA_SHIFT, CFG1_IA_BITS) + 1;
    cpuinfo.ic_nsets = 1 << (_mips32r2_ext(config1, CFG1_IS_SHIFT, CFG1_IS_BITS) + 6);
    cpuinfo.ic_size = cpuinfo.ic_nways * cpuinfo.ic_linesize * cpuinfo.ic_nsets;

    /* Data cache size and organization. */
    cpuinfo.dc_linesize = (config1 & CFG1_DL_MASK) ? 32 : 0;
    cpuinfo.dc_nways = _mips32r2_ext(config1, CFG1_DA_SHIFT, CFG1_DA_BITS) + 1;
    cpuinfo.dc_nsets = 1 << (_mips32r2_ext(config1, CFG1_DS_SHIFT, CFG1_DS_BITS) + 6);
    cpuinfo.dc_size = cpuinfo.dc_nways * cpuinfo.dc_linesize * cpuinfo.dc_nsets;

    kprintf("TLB Entries: %d\n", cpuinfo.tlb_entries);

    kprintf("I-cache: %d KiB, %d-way associative, line size: %d bytes\n",
            cpuinfo.ic_size / 1024, cpuinfo.ic_nways, cpuinfo.ic_linesize);
    kprintf("D-cache: %d KiB, %d-way associative, line size: %d bytes\n",
            cpuinfo.dc_size / 1024, cpuinfo.dc_nways, cpuinfo.dc_linesize);

    /* Config2 implemented? */
    if ((config1 & CFG1_M) == 0)
        return 0;

    uint32_t config2 = mips32_getconfig1();

    /* Config3 implemented? */
    if ((config2 & CFG2_M) == 0)
        return 0;

    uint32_t config3 = mips32_getconfig3();

    kprintf("Vectored interrupts implemented : %s\n", (config3 & CFG3_VI) ? "yes" : "no");
    kprintf("EIC interrupt mode implemented    : %s\n", (config3 & CFG3_VEIC) ? "yes" : "no");

    uint32_t coreid_rev = *((volatile uint32_t *)MIPS_PHYS_TO_KSEG1(MALTA_REVISION));
    coreid_rev >>= MALTA_REVISION_CORID_SHF;
    coreid_rev &= (MALTA_REVISION_CORID_MSK >> MALTA_REVISION_CORID_SHF);

    assert(MALTA_REVISION_CORID_CORE_LV == coreid_rev);
    kprintf("CORID Rev: CoreLV\n");

    return 1;
}

/* Print state of control registers. */
void platform_dump_additional_cpu_info(void)
{
    unsigned cr = mips32_get_c0(C0_CAUSE);
    unsigned sr = mips32_get_c0(C0_STATUS);
    unsigned intctl = mips32_get_c0(C0_INTCTL);
    unsigned srsctl = mips32_get_c0(C0_SRSCTL);

  kprintf ("Cause    : TI:%d IV:%d IP:%d ExcCode:%d\n",
           (cr & CR_TI) >> CR_TI_SHIFT,
           (cr & CR_IV) >> CR_IV_SHIFT,
           (cr & CR_IP_MASK) >> CR_IP_SHIFT,
           (cr & CR_X_MASK) >> CR_X_SHIFT);
  kprintf ("Status   : CU0:%d BEV:%d NMI:%d IM:$%02x KSU:%d ERL:%d EXL:%d IE:%d\n",
           (sr & SR_CU0) >> SR_CU0_SHIFT,
           (sr & SR_BEV) >> SR_BEV_SHIFT,
           (sr & SR_NMI) >> SR_NMI_SHIFT,
           (sr & SR_IMASK) >> SR_IMASK_SHIFT,
           (sr & SR_KSU_MASK) >> SR_KSU_SHIFT,
           (sr & SR_ERL) >> SR_ERL_SHIFT,
           (sr & SR_EXL) >> SR_EXL_SHIFT,
           (sr & SR_IE) >> SR_IE_SHIFT);
  kprintf ("IntCtl   : IPTI:%d IPPCI:%d VS:%d\n",
           (intctl & INTCTL_IPTI) >> INTCTL_IPTI_SHIFT,
           (intctl & INTCTL_IPPCI) >> INTCTL_IPPCI_SHIFT,
           (intctl & INTCTL_VS) >> INTCTL_VS_SHIFT);
  kprintf ("SrsCtl   : HSS:%d\n",
           (srsctl & SRSCTL_HSS) >> SRSCTL_HSS_SHIFT);
  kprintf ("EPC      : $%08x\n", (unsigned)mips32_get_c0(C0_EPC));
  kprintf ("ErrPC    : $%08x\n", (unsigned)mips32_get_c0(C0_ERRPC));
  kprintf ("EBase    : $%08x\n", (unsigned)mips32_get_c0(C0_EBASE));
}

uintptr_t platform_iomem_phy_to_virt(uintptr_t iomem_phy)
{
    return MIPS_PHYS_TO_KSEG1(iomem_phy);
}

uintptr_t platform_iomem_virt_to_phy(uintptr_t iomem_virt)
{
    return MIPS_KSEG1_TO_PHYS(iomem_virt);
}

uintptr_t platform_ioport_to_phy(uintptr_t ioport)
{
    return MALTA_PCI0_IO_ADDR(ioport);
}

uintptr_t platform_ioport_to_virt(uintptr_t ioport)
{
    return MIPS_PHYS_TO_KSEG1(platform_ioport_to_phy(ioport));
}

uintptr_t platform_virt_to_phy(uintptr_t virt)
{
    return MIPS_KSEG0_TO_PHYS(virt);
}

uintptr_t platform_phy_to_virt(uintptr_t phy)
{
    return MIPS_PHYS_TO_KSEG0(phy);
}

uintptr_t platform_buffered_virt_to_unbuffered_virt(uintptr_t virt)
{
    return platform_iomem_phy_to_virt(platform_virt_to_phy(virt));
}

int platform_cpu_dcacheline_size(void)
{
    return cpuinfo.dc_linesize;
}

int platform_cpu_icacheline_size(void)
{
    return cpuinfo.ic_linesize;
}

void platform_cpu_wait(void)
{
	asm volatile("nop\nnop\nnop\nwait\nnop\nnop\nnop\n");
}
