//
// Created by gz on 7/8/16.
//

#include <piix4.h>
#include <platform/kprintf.h>
#include <cstdio>
#include "piix4_regs.h"

DECLARE_PCI_DRIVER(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB, piix4_ide, piix4_ide_pci_probe);
DECLARE_PCI_DRIVER(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0, piix4_isa, piix4_isa_pci_probe);
DECLARE_PCI_DRIVER(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3, piix4_acpi, piix4_acpi_pci_probe);

/* PCI interrupt pins */
#define PCIA        1
#define PCIB        2
#define PCIC        3
#define PCID        4

/* This table is filled in by interrogating the PIIX4 chip */
static char pci_irq[5] = { 0 };
static char irq_tab[][5] = {
        	/*  	INTA    INTB    INTC    INTD */
            {0,     0,  	0,  	0,  	0 },    /*  0: GT64120 PCI bridge */
            {0,     0,  	0,  	0,  	0 },    /*  1: Unused */
            {0,     0,  	0,  	0,  	0 },    /*  2: Unused */
            {0, 	0,  	0,  	0,  	0 },    /*  3: Unused */
            {0, 	0,  	0,  	0,  	0 },    /*  4: Unused */
            {0, 	0,  	0,  	0,  	0 },    /*  5: Unused */
            {0, 	0,  	0,  	0,  	0 },    /*  6: Unused */
            {0, 	0,  	0,  	0,  	0 },    /*  7: Unused */
            {0, 	0,  	0,  	0,  	0 },    /*  8: Unused */
            {0, 	0,  	0,  	0,  	0 },    /*  9: Unused */
            {0, 	0,  	0,  	0,  	PCID }, /* 10: PIIX4 USB */
            {0, 	PCIB,   0,  	0,  	0 },    /* 11: AMD 79C973 Ethernet */
            {0, 	PCIC,   0,  	0,  	0 },    /* 12: Crystal 4281 Sound */
            {0, 	0,  	0,  	0,  	0 },    /* 13: Unused */
            {0, 	0,  	0,  	0,  	0 },    /* 14: Unused */
            {0, 	0,  	0,  	0,  	0 },    /* 15: Unused */
            {0, 	0,  	0,  	0,  	0 },    /* 16: Unused */
            {0, 	0,  	0,  	0,  	0 },    /* 17: Bonito/SOC-it PCI Bridge*/
            {0, 	PCIA,   PCIB,   PCIC,   PCID }, /* 18: PCI Slot 1 */
            {0, 	PCIB,   PCIC,   PCID,   PCIA }, /* 19: PCI Slot 2 */
            {0, 	PCIC,   PCID,   PCIA,   PCIB }, /* 20: PCI Slot 3 */
            {0, 	PCID,   PCIA,   PCIB,   PCIC }  /* 21: PCI Slot 4 */
};

extern "C"
int platform_pci_map_irq(uint8_t slot, uint8_t pin)
{
    int virq;
    virq = irq_tab[slot][pin];
//    printf("map_irq: slot %d pin %d virq %d\n", slot, pin, virq);
    return pci_irq[virq];
}

static void malta_piix_func0_fixup(PCIDevice* pci_dev) {
    unsigned char reg_val;
    /* PIIX PIRQC[A:D] irq mappings */
    static int piixirqmap[16] = {
            0, 0, 0, 3,
            4, 5, 6, 7,
            0, 9, 10, 11,
            12, 0, 14, 15
    };
    int i;

    /* Interrogate PIIX4 to get PCI IRQ mapping */
    for (i = 0; i <= 3; i++) {
        reg_val = pci_dev->readByte(PIIX4_FUNC0_PIRQRC + i);
        if (reg_val & PIIX4_FUNC0_PIRQRC_IRQ_ROUTING_DISABLE)
            pci_irq[PCIA + i] = 0;    /* Disabled */
        else
            pci_irq[PCIA + i] = (char) piixirqmap[reg_val & PIIX4_FUNC0_PIRQRC_IRQ_ROUTING_MASK];
    }
}

extern "C"
int piix4_isa_pci_probe(PCIDevice* pci_dev)
{
    PCIBus* pci_bus = platform_pci_bus();

    /* setup PCI interrupt routing */
    malta_piix_func0_fixup(pci_dev);
    for (unsigned int j = PCIA; j < sizeof(pci_irq); j++) {
        //kprintf("[pci] routing PIN-%c to IRQ%d\n", 'A' + j - 1, pci_irq[j]);
        pci_dev->writeByte(PIIX4_FUNC0_PIRQRC + j - 1, (uint8_t) pci_irq[j]);
    }

    for (PCIDevice& dev : pci_bus->devices()) {
        if (0 == dev.pin()) {
            continue;
        }

        uint8_t irq = dev.irq();
        uint8_t pin = dev.pin();
        uint8_t slot = (uint8_t) dev.dev();
        uint8_t newIrq = (uint8_t) platform_pci_map_irq(slot, pin);
        if (newIrq == irq) {
            continue;
        }

        dev.setIrq(newIrq);
        dev.writeByte(PCI_INTERRUPT_LINE, dev.irq());
        /*kprintf("[pci] rerouting IRQ%d to IRQ%d for [pci:%02x:%02x.%02x]\n",
                irq,
                dev.irq(),
                dev.bus(), dev.dev(), dev.devfunc());*/
    }

    /* Done by YAMON 2.00 onwards */
    if (PCI_SLOT(pci_dev->devfunc()) == 10) {
        /*
         * Set top of main memory accessible by ISA or DMA
         * devices to 16 Mb.
         */
        uint8_t val = pci_dev->readByte(PIIX4_FUNC0_TOM);
        val |= PIIX4_FUNC0_TOM_TOP_OF_MEMORY_MASK;
        pci_dev->writeByte(PIIX4_FUNC0_TOM, val);
    }

    /* Mux SERIRQ to its pin */
    uint32_t gencfg = pci_dev->readWord(PIIX4_FUNC0_GENCFG);
    gencfg |= PIIX4_FUNC0_GENCFG_SERIRQ;
    pci_dev->writeWord(PIIX4_FUNC0_GENCFG, gencfg);

    /* Enable SERIRQ */
    uint8_t serirqc = pci_dev->readByte(PIIX4_FUNC0_SERIRQC);
    serirqc |= (PIIX4_FUNC0_SERIRQC_EN | PIIX4_FUNC0_SERIRQC_CONT);
    pci_dev->writeByte(PIIX4_FUNC0_SERIRQC, serirqc);

    /* Enable response to special cycles */
    uint16_t cmd = pci_dev->readHalf(PCI_COMMAND);
    cmd |= PCI_COMMAND_SPECIAL;
    pci_dev->writeHalf(PCI_COMMAND, cmd);

    uint8_t odlc = pci_dev->readByte(PIIX4_FUNC0_DLC);
    /* Enable passive releases and delayed transaction */
    odlc |= (PIIX4_FUNC0_DLC_USBPR_EN | PIIX4_FUNC0_DLC_PASSIVE_RELEASE_EN | PIIX4_FUNC0_DLC_DELAYED_TRANSACTION_EN);
    pci_dev->writeByte(PIIX4_FUNC0_DLC, odlc);

    return 0;
}

extern "C"
int piix4_ide_pci_probe(PCIDevice* pci_dev)
{
    /* enable bus master & IO access */
    uint16_t val16 = PCI_COMMAND_MASTER | PCI_COMMAND_IO;
    pci_dev->writeHalf(PCI_COMMAND, val16);

    uint16_t state = pci_dev->readHalf(PCI_COMMAND);
    if (!(state & PCI_COMMAND_MASTER)) {
        kprintf("Failed to apply pci master. expected %04x got %04x\n", val16, state);
        return 1;
    }

    /* set latency */
    pci_dev->writeByte(PCI_LATENCY_TIMER, 0x40);

    unsigned char reg_val;

    /* Done by YAMON 2.02 onwards */
    if (PCI_SLOT(pci_dev->devfunc()) == 10) {
        /*
         * IDE Decode enable.
         */
        reg_val = pci_dev->readByte(PIIX4_FUNC1_IDETIM_PRIMARY_HI);
        reg_val |= PIIX4_FUNC1_IDETIM_PRIMARY_HI_IDE_DECODE_EN;
        pci_dev->writeByte(PIIX4_FUNC1_IDETIM_PRIMARY_HI, reg_val);

        reg_val = pci_dev->readByte(PIIX4_FUNC1_IDETIM_SECONDARY_HI);
        reg_val |= PIIX4_FUNC1_IDETIM_SECONDARY_HI_IDE_DECODE_EN;
        pci_dev->writeByte(PIIX4_FUNC1_IDETIM_SECONDARY_HI, reg_val);
    }

    return 0;
}

extern "C"
int piix4_acpi_pci_probe(PCIDevice* pci_dev)
{
    /* Set a sane PM I/O base address */
    pci_dev->writeHalf(PIIX4_FUNC3_PMBA, 0x1000);

    /* Enable access to the PM I/O region */
    pci_dev->writeByte(PIIX4_FUNC3_PMREGMISC, PIIX4_FUNC3_PMREGMISC_EN);
    return 0;
}
