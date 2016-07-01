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
           (sr & SR_EXL) >> SR_ERL_SHIFT,
           (sr & SR_IE) >> SR_ERL_SHIFT);
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

#define GT_ADDR_REG(reg)    (uint32_t) (GT_CFGADDR_CFGEN_BIT | \
                            (0 << GT_CFGADDR_BUSNUM_SHF) | \
                            (0 << GT_CFGADDR_DEVNUM_SHF) | \
                            (0 << GT_CFGADDR_FUNCNUM_SHF) | \
                            (((reg) >> 2) << GT_CFGADDR_REGNUM_SHF))

void platform_init_bar_registers(void)
{
    uint32_t gt_default_base = MIPS_PHYS_TO_KSEG1(GT_DEF_BASE);
    uint32_t gt_new_base = MIPS_PHYS_TO_KSEG1(MALTA_CORECTRL_BASE);

    kprintf("GT_ISD_OFS      %08x\n", read32(gt_new_base, GT_ISD_OFS));
    kprintf("GT_PCI0IOLD_OFS %08x\n", read32(gt_new_base, GT_PCI0IOLD_OFS));
    kprintf("GT_PCI0IOHD_OFS %08x\n", read32(gt_new_base, GT_PCI0IOHD_OFS));
    kprintf("GT_PCI0M0LD_OFS %08x\n", read32(gt_new_base, GT_PCI0M0LD_OFS));
    kprintf("GT_PCI0M0HD_OFS %08x\n", read32(gt_new_base, GT_PCI0M0HD_OFS));
    kprintf("GT_PCI0M1LD_OFS %08x\n", read32(gt_new_base, GT_PCI0M1LD_OFS));
    kprintf("GT_PCI0M1HD_OFS %08x\n", read32(gt_new_base, GT_PCI0M1HD_OFS));

    /* move GT64120 registers from 0x14000000 to 0x1be00000 */
    write32(gt_default_base, GT_ISD_OFS, cpu_to_le32(MALTA_CORECTRL_BASE >> 21));

    /* Setup GT64120 CPU interface */
    uint32_t gt_cpu = read32(gt_new_base, GT_CPU_OFS);
    gt_cpu &= ~GT_CPU_WR_MSK;
    gt_cpu |= GT_CPU_WR_DDDD << GT_CPU_WR_SHF;
    write32(gt_new_base, GT_CPU_OFS, gt_cpu);

    /*  HW bug workaround: Extend BootCS mapping area to access FPGA without
    *             4 x access on CBUS per 1 x access from SysAD.
    */
    write32(gt_new_base, GT_CS3HD_OFS, 0);
    write32(gt_new_base, GT_BOOTLD_OFS, cpu_to_le32(0xf0));
    write32(gt_new_base, GT_BOOTHD_OFS, cpu_to_le32(0xff));

    /* Setup byte/word swap */
#if BYTE_ORDER == BIG_ENDIAN
    write32(gt_new_base, GT_PCI0_CMD_OFS, cpu_to_le32(0x00401));
#elif BYTE_ORDER == LITTLE_ENDIAN
    write32(gt_new_base, GT_PCI0_CMD_OFS, cpu_to_le32(0x10001));
#else
    #error "Unknown system"
#endif

    /* Change retrycount to a value, which is not 0 */
    write32(gt_new_base, GT_PCI0_TOR_OFS, cpu_to_le32(0x00ffffff));

    /* Setup GT64120 to have Master capability */
    write32(gt_new_base, GT_PCI0_CFGADDR_OFS, GT_ADDR_REG(PCI_SC));
    uint32_t pci_sc = read32(gt_new_base, GT_PCI0_CFGDATA_OFS);
    pci_sc |= (PCI_SC_CMD_MS_BIT | PCI_SC_CMD_BM_BIT | PCI_SC_CMD_PERR_BIT | PCI_SC_CMD_SERR_BIT);

    write32(gt_new_base, GT_PCI0_CFGADDR_OFS, GT_ADDR_REG(PCI_SC));
    write32(gt_new_base, GT_PCI0_CFGDATA_OFS, pci_sc);

    /* Setup GT64120 PCI latency timer */
    write32(gt_new_base, GT_PCI0_CFGADDR_OFS, GT_ADDR_REG(PCI_BHLC));
    uint32_t pci_bhlc = read32(gt_new_base, GT_PCI0_CFGDATA_OFS);
    pci_bhlc |= (GT_LATTIM_MIN << PCI_BHLC_LT_SHF);

    write32(gt_new_base, GT_PCI0_CFGADDR_OFS, GT_ADDR_REG(PCI_BHLC));
    write32(gt_new_base, GT_PCI0_CFGDATA_OFS, pci_bhlc);

    /* setup PCI0 io window to 0x18000000-0x181fffff */
    write32(gt_new_base, GT_PCI0IOLD_OFS, cpu_to_le32(0x18000000 >> 21));
    write32(gt_new_base, GT_PCI0IOHD_OFS, 0x40000000);

    /* setup PCI0 mem windows */
    write32(gt_new_base, GT_PCI0M0LD_OFS, 0x80000000);
    write32(gt_new_base, GT_PCI0M0HD_OFS, 0x3f000000);
    write32(gt_new_base, GT_PCI0M1LD_OFS, 0xc1000000);
    write32(gt_new_base, GT_PCI0M1HD_OFS, 0x5e000000);
}