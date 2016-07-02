//
// Created by gz on 7/2/16.
//

#include <lib/drivers/pcnet/pcnet.h>
#include <platform/pci/pci.h>
#include <platform/kprintf.h>

DECLARE_PCI_DRIVER(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_LANCE, pcnet, pcnet_pci_probe);

extern "C"
int pcnet_pci_probe(pci_device_t* pci_dev)
{
    kprintf("in pcnet_pci_probe!\n");
    return 0;
}