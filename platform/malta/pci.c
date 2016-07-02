#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <platform/malta/pci.h>
#include <platform/malta/gt64120.h>
#include <platform/malta/mips.h>
#include <platform/sbrk.h>
#include <platform/kprintf.h>
#include <platform/cpu.h>
#include <platform/pci/pci.h>

#define PCI0_CFG_ADDR_R GT_R(GT_PCI0_CFG_ADDR)
#define PCI0_CFG_DATA_R GT_R(GT_PCI0_CFG_DATA)

#define PCI0_CFG_REG_SHIFT   2
#define PCI0_CFG_FUNCT_SHIFT 8
#define PCI0_CFG_DEV_SHIFT   11
#define PCI0_CFG_BUS_SHIFT   16
#define PCI0_CFG_ENABLE      0x80000000

#define PCI0_CFG_REG(dev, funct, reg)  \
  (((dev) << PCI0_CFG_DEV_SHIFT)     | \
   ((funct) << PCI0_CFG_FUNCT_SHIFT) | \
   ((reg) << PCI0_CFG_REG_SHIFT))

/* For reference look at: http://wiki.osdev.org/PCI */

uint32_t platform_pci_bus_read_word(int dev, int devfn, int reg)
{
    PCI0_CFG_ADDR_R = cpu_to_le32(PCI0_CFG_ENABLE | PCI0_CFG_REG(dev, devfn, reg));
    return PCI0_CFG_DATA_R;
}

void platform_pci_bus_write_word(int dev, int devfn, int reg, uint32_t value)
{
    PCI0_CFG_ADDR_R = cpu_to_le32(PCI0_CFG_ENABLE | PCI0_CFG_REG(dev, devfn, reg));
    PCI0_CFG_DATA_R = value;
}

uint32_t platform_pci_dev_read_word(const pci_device_t* pcidev, int reg)
{
    return platform_pci_bus_read_word(pcidev->addr.device, pcidev->addr.function, reg);
}

void platform_pci_dev_write_word(const pci_device_t *pcidev, int reg, uint32_t value)
{
    platform_pci_bus_write_word(pcidev->addr.device, pcidev->addr.function, reg, value);
}

void platform_pci_bus_enumerate(pci_bus_t *pcibus)
{
    pcibus->dev = kernel_sbrk(0);
    pcibus->ndevs = 0;

    for (int dev = 0; dev < 32; dev++) {
        for (int devfn = 0; devfn < 8; devfn++) {
            PCI0_CFG_ADDR_R = cpu_to_le32(PCI0_CFG_ENABLE | PCI0_CFG_REG(dev, devfn, 0));

            if (PCI0_CFG_DATA_R == -1) {
                continue;
            }

            pci_device_t *pcidev = kernel_sbrk(sizeof(pci_device_t));

            pcidev->addr.bus = 0;
            pcidev->addr.device = (uint8_t) dev;
            pcidev->addr.function = (uint8_t) devfn;

            uint32_t device_vendor = (PCI0_CFG_DATA_R);
            if (0 == dev && 0 == devfn) {
                device_vendor = le32_to_cpu(device_vendor);
            }

            pcidev->device_id = (uint16_t) (device_vendor >> 16);
            pcidev->vendor_id = (uint16_t) (device_vendor);

            PCI0_CFG_ADDR_R = cpu_to_le32(PCI0_CFG_ENABLE | PCI0_CFG_REG(dev, devfn, 2));
            uint32_t class_code = (PCI0_CFG_DATA_R);
            if (0 == dev && 0 == devfn) {
                class_code = le32_to_cpu(class_code);
            }

            pcidev->class_code = (uint8_t) ((class_code & 0xff000000) >> 24);

            PCI0_CFG_ADDR_R = cpu_to_le32(PCI0_CFG_ENABLE | PCI0_CFG_REG(dev, devfn, 15));
            uint32_t pin_and_irq = PCI0_CFG_DATA_R;
            if (0 == dev && 0 == devfn) {
                pin_and_irq = le32_to_cpu(pin_and_irq);
            }

            pcidev->pin = (uint8_t) ((pin_and_irq >> 8));
            pcidev->irq = (uint8_t) (pin_and_irq);

            for (int i = 0; i < 6; i++) {
                PCI0_CFG_ADDR_R = cpu_to_le32(
                        PCI0_CFG_ENABLE | PCI0_CFG_REG(pcidev->addr.device, pcidev->addr.function, 4 + i));
                uint32_t addr = PCI0_CFG_DATA_R;
                PCI0_CFG_DATA_R = (uint32_t) -1;
                uint32_t size = PCI0_CFG_DATA_R;
                if (size == 0 || addr == size) {
                    continue;
                }

                size &= (addr & PCI_BAR_IO) ? ~PCI_BAR_IO_MASK : ~PCI_BAR_MEMORY_MASK;
                size = (uint32_t) -size;

                pci_bar_t *bar = &pcidev->bar[pcidev->nbars++];
                bar->phy_addr = addr;
                bar->size = size;
            }

            pcibus->ndevs++;
        }
    }
}

intptr_t platform_pci_memory_base(void)
{
    return MALTA_PCI0_MEMORY_BASE;
}

intptr_t platform_pci_io_base(void)
{
    return PCI_IO_SPACE_BASE;
}
