//
// Created by gz on 7/2/16.
//

#ifndef GZOS_PCNET_H
#define GZOS_PCNET_H

#include <platform/pci/pci.h>

#ifdef __cplusplus
extern "C" {
#endif

int pcnet_pci_probe(pci_device_t* pci_dev);

#ifdef __cplusplus
}
#endif
#endif //GZOS_PCNET_H
