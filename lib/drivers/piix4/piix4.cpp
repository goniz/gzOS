//
// Created by gz on 7/8/16.
//

#include <lib/drivers/piix4/piix4.h>
#include <platform/malta/malta.h>
#include <platform/kprintf.h>

DECLARE_PCI_DRIVER(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0, piix4_isa, piix4_isa_pci_probe);
DECLARE_PCI_DRIVER(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB, piix4_ide, piix4_ide_pci_probe);

extern "C"
int piix4_isa_pci_probe(PCIDevice* pci_dev)
{
    /* setup PCI interrupt routing */
    pci_dev->writeWord(PCI_CFG_PIIX4_PIRQRCA, 10);
    pci_dev->writeWord(PCI_CFG_PIIX4_PIRQRCB, 10);
    pci_dev->writeWord(PCI_CFG_PIIX4_PIRQRCC, 11);
    pci_dev->writeWord(PCI_CFG_PIIX4_PIRQRCD, 11);

    /* mux SERIRQ onto SERIRQ pin */
    uint32_t val32 = pci_dev->readWord(PCI_CFG_PIIX4_GENCFG);
    val32 |= PCI_CFG_PIIX4_GENCFG_SERIRQ;
    pci_dev->writeWord(PCI_CFG_PIIX4_GENCFG, val32);

    /* enable SERIRQ - Linux currently depends upon this */
    uint8_t val8 = pci_dev->readByte(PCI_CFG_PIIX4_SERIRQC);
    val8 |= PCI_CFG_PIIX4_SERIRQC_EN | PCI_CFG_PIIX4_SERIRQC_CONT;
    pci_dev->writeByte(PCI_CFG_PIIX4_SERIRQC, val8);

    return 0;
}

extern "C"
int piix4_ide_pci_probe(PCIDevice* pci_dev)
{
    /* enable bus master & IO access */
    uint32_t val32 = PCI_COMMAND_MASTER | PCI_COMMAND_IO;
    pci_dev->writeWord(PCI_COMMAND, val32);

    uint32_t state = pci_dev->readWord(PCI_COMMAND);
    if (!(state & PCI_COMMAND_MASTER)) {
        kprintf("Failed to apply pci master. expected %08x got %08x\n", val32, state);
        return 1;
    }

    /* set latency */
    pci_dev->writeByte(PCI_LATENCY_TIMER, 0x40);

    /* enable IDE/ATA */
    pci_dev->writeWord(PCI_CFG_PIIX4_IDETIM_PRI, PCI_CFG_PIIX4_IDETIM_IDE);
    pci_dev->writeWord(PCI_CFG_PIIX4_IDETIM_SEC, PCI_CFG_PIIX4_IDETIM_IDE);

    return 0;
}