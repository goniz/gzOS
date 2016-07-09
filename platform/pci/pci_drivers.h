//
// Created by gz on 7/2/16.
//

#ifndef GZOS_PCI_DRIVERS_H_H
#define GZOS_PCI_DRIVERS_H_H

#include <platform/pci/pci.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PCIDevice;
typedef int (*pci_driver_probe_func_t)(struct PCIDevice* pci_dev);

struct pci_driver_ent {
    uint16_t vendor_id;
    uint16_t device_id;
    const char* driver_name;
    pci_driver_probe_func_t probe_func;
};

#define DECLARE_PCI_DRIVER(vendor_id, device_id, driver_name, probe_func) \
            __attribute__((section(".pci_drivers"),used)) \
            static struct pci_driver_ent __pci_drv_ ##driver_name = { (vendor_id), (device_id), #driver_name, (pci_driver_probe_func_t)(probe_func) }

#ifdef __cplusplus
}
#endif
#endif //GZOS_PCI_DRIVERS_H_H
