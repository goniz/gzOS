//
// Created by gz on 7/2/16.
//

#include <lib/drivers/pcnet/pcnet.h>
#include <platform/pci/pci.h>
#include <platform/kprintf.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <vector>
#include <platform/malta/mips.h>
#include <platform/malta/gt64120.h>
#include <platform/cpu.h>
#include <platform/malta/malta.h>

DECLARE_PCI_DRIVER(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_LANCE, pcnet, pcnet_pci_probe);
static std::vector<pcnet_drv> __pcnet_devices;

static std::atomic<int> __dev_index(0);
static int generate_dev_index(void)
{
    return __dev_index.fetch_add(1, std::memory_order::memory_order_relaxed);
}

extern "C"
int pcnet_pci_probe(pci_device_t* pci_dev)
{
    pcnet_drv drv(pci_dev);

    kprintf("dev name: %s\n", drv.name());
    kprintf("pci bus %d dev %d devfn %d iobase %x\n",
            pci_dev->addr.bus,
            pci_dev->addr.device,
            pci_dev->addr.function,
            platform_pci_device_get_iobase(pci_dev));

    __pcnet_devices.push_back(std::move(drv));
    return 0;
}

pcnet_drv::pcnet_drv(pci_device_t *pci_dev)
    : _pcidev(pci_dev),
      _iobase(platform_pci_device_get_iobase(pci_dev))
{
    sprintf(_name, "pcnet%d", generate_dev_index());

    kprintf("test %08x\n", platform_pci_dev_read_word(_pcidev, PCI_BASE_ADDRESS_0));

    uint32_t command = cpu_to_le32(PCI_COMMAND_IO | PCI_COMMAND_MASTER);
    kprintf("command %08x\n", command);
    platform_pci_dev_write_word(_pcidev, PCI_COMMAND, command);

    uint32_t status = platform_pci_dev_read_word(_pcidev, PCI_COMMAND);
    kprintf("status %08x\n", status);
    if ((status & command) != command) {
        kprintf("Could not enable IO access or Bus Mastering..\n");
    }

    platform_pci_dev_write_word(_pcidev, PCI_LATENCY_TIMER, 0x40);
}

pcnet_drv::pcnet_drv(pcnet_drv &&other)
    : _pcidev(nullptr),
      _iobase(0)
{
    std::swap(_pcidev, other._pcidev);
    std::swap(_iobase, other._iobase);
    strncpy(_name, other._name, 16);
    memset(other._name, 0, sizeof(other._name));
}



