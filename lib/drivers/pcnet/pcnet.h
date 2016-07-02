//
// Created by gz on 7/2/16.
//

#ifndef GZOS_PCNET_H
#define GZOS_PCNET_H

#include <platform/pci/pci.h>

#ifdef __cplusplus

class pcnet_drv
{
public:
    pcnet_drv(pci_device_t* pci_dev);
    pcnet_drv(pcnet_drv&& other);

    const char* name(void) const {
        return _name;
    }

private:
    char _name[16];
    pci_device_t* _pcidev;
    uintptr_t _iobase;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

int pcnet_pci_probe(pci_device_t* pci_dev);

#ifdef __cplusplus
}
#endif
#endif //GZOS_PCNET_H
