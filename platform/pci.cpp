//
// Created by gz on 7/2/16.
//
#include <platform/pci.h>
#include <platform/kprintf.h>
#include <platform/sbrk.h>
#include <cstdlib>
#include <cstdio>

static pci_bus_t pci_bus;

extern "C"
void platform_pci_init(void)
{
    intptr_t pci_memory_base = platform_pci_memory_base();
    intptr_t pci_io_base = platform_pci_io_base();

    platform_pci_bus_enumerate(&pci_bus);
    platform_pci_bus_assign_space(&pci_bus, pci_memory_base, pci_io_base);
    platform_pci_bus_dump(&pci_bus);
}

extern "C"
void platform_pci_driver_probe()
{
    for (unsigned int j = 0; j < pci_bus.ndevs; j++) {
//        pci_device_t *pcidev = &pci_bus.dev[j];
        
    }
}

static int pci_bar_compare(const void *a, const void *b)
{
    const pci_bar_t *bar0 = *(const pci_bar_t **) a;
    const pci_bar_t *bar1 = *(const pci_bar_t **) b;

    if (bar0->size < bar1->size) {
        return 1;
    }
    if (bar0->size > bar1->size) {
        return -1;
    }
    return 0;
}

extern "C"
void platform_pci_bus_assign_space(pci_bus_t *pcibus, intptr_t mem_base, intptr_t io_base)
{
    pci_bar_t **bars = (pci_bar_t **) kernel_sbrk(0);
    unsigned nbars = 0;

    for (unsigned int j = 0; j < pcibus->ndevs; j++) {
        pci_device_t *pcidev = &pcibus->dev[j];

        for (unsigned int i = 0; i < pcidev->nbars; i++) {
            void *ptr __attribute__((unused));
            ptr = kernel_sbrk(sizeof(pci_bar_t *));
            bars[nbars++] = &pcidev->bar[i];
        }
    }

    qsort(bars, nbars, sizeof(pci_bar_t *), pci_bar_compare);

    for (unsigned int j = 0; j < nbars; j++) {
        pci_bar_t *bar = bars[j];
        if (bar->phy_addr & PCI_BAR_IO) {
            bar->phy_addr |= io_base;
            io_base += bar->size;
        } else {
            bar->phy_addr |= mem_base;
            mem_base += bar->size;
        }
    }

    kernel_brk(bars);
}

static const pci_device_id *pci_find_device(const pci_vendor_id *vendor,
                                            uint16_t device_id) {
    if (vendor) {
        const pci_device_id *device = vendor->devices;
        while (device->name) {
            if (device->id == device_id)
                return device;
            device++;
        }
    }
    return NULL;
}

static const pci_vendor_id *pci_find_vendor(uint16_t vendor_id) {
    const pci_vendor_id *vendor = pci_vendor_list;
    while (vendor->name) {
        if (vendor->id == vendor_id)
            return vendor;
        vendor++;
    }
    return NULL;
}

extern "C"
void platform_pci_dev_dump(const pci_device_t* pcidev)
{
    char devstr[32];

    sprintf(devstr, "[pci:%02x:%02x.%02x]",
            pcidev->addr.bus,
            pcidev->addr.device,
            pcidev->addr.function);

    const pci_vendor_id *vendor =
            pci_find_vendor(pcidev->vendor_id);
    const pci_device_id *device =
            pci_find_device(vendor, pcidev->device_id);

    kprintf("%s %s", devstr, pci_class_code[pcidev->class_code]);

    if (vendor)
        kprintf(" %s", vendor->name);
    else
        kprintf(" vendor:$%04x", pcidev->vendor_id);

    if (device)
        kprintf(" %s\n", device->name);
    else
        kprintf(" device:$%04x\n", pcidev->device_id);

    if (pcidev->pin)
        kprintf("%s Interrupt: pin %c routed to IRQ %d\n",
                devstr, 'A' + pcidev->pin - 1, pcidev->irq);

    for (unsigned int i = 0; i < pcidev->nbars; i++) {
        const pci_bar_t *bar = &pcidev->bar[i];
        uint32_t addr = bar->phy_addr;
        size_t size = bar->size;
        const char* type = nullptr;

        if (addr & PCI_BAR_IO) {
            addr &= ~PCI_BAR_IO_MASK;
            type = "I/O ports";
        } else {
            if (addr & PCI_BAR_PREFETCHABLE)
                type = "Memory (prefetchable)";
            else
                type = "Memory (non-prefetchable)";
            addr &= ~PCI_BAR_MEMORY_MASK;
        }
        kprintf("%s Region %d: %s at %p [size=$%x]\n",
                devstr, i, type, (void *) addr, (unsigned) size);
    }
}

extern "C"
void platform_pci_bus_dump(const pci_bus_t *pcibus)
{
    kprintf("PCI devices: %d\n", pcibus->ndevs);

    for (unsigned int j = 0; j < pcibus->ndevs; j++) {
        pci_device_t *pcidev = &pcibus->dev[j];
        platform_pci_dev_dump(pcidev);
    }
}