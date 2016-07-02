//
// Created by gz on 7/2/16.
//
/* Please read http://wiki.osdev.org/PCI */

#ifndef GZOS_PCI_H
#define GZOS_PCI_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PCI_IO_SPACE_BASE     0x1000

#define PCI_BAR_MEMORY        0
#define PCI_BAR_IO            1
#define PCI_BAR_64BIT         4
#define PCI_BAR_PREFETCHABLE  8

#define PCI_BAR_IO_MASK       3
#define PCI_BAR_MEMORY_MASK  15

typedef struct {
    uint32_t phy_addr;
    size_t size;
} pci_bar_t;

typedef struct {
    struct {
        uint8_t bus;
        uint8_t device;
        uint8_t function;
    } addr;

    uint16_t device_id;
    uint16_t vendor_id;
    uint8_t  class_code;
    uint8_t  pin, irq;

    unsigned  nbars;
    pci_bar_t bar[6];
} pci_device_t;

typedef struct {
    unsigned ndevs;
    pci_device_t *dev;
} pci_bus_t;

void platform_pci_init(void);
void platform_pci_driver_probe(void);
void platform_pci_bus_dump(const pci_bus_t *pcibus);
void platform_pci_bus_assign_space(pci_bus_t *pcibus, intptr_t mem_base, intptr_t io_base);

uint32_t platform_pci_bus_read_word(int dev, int devfn, int reg);
void platform_pci_bus_write_word(int dev, int devfn, int reg, uint32_t value);
uint32_t platform_pci_dev_read_word(const pci_device_t* pcidev, int reg);
void platform_pci_dev_write_word(const pci_device_t *pcidev, int reg, uint32_t value);

void platform_pci_bus_enumerate(pci_bus_t *pcibus);
intptr_t platform_pci_memory_base(void);
intptr_t platform_pci_io_base(void);
uintptr_t platform_pci_device_get_iobase(pci_device_t* pcidev);

#ifdef __cplusplus
}
#endif

#include <platform/pci/pci_drivers.h>
#include <platform/pci/pci_commands.h>
#include <platform/pci/pci_ids.h>
#endif //GZOS_PCI_H
