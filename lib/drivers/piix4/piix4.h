//
// Created by gz on 7/8/16.
//

#ifndef GZOS_PIXX4_ISA_H
#define GZOS_PIXX4_ISA_H

#include <platform/pci/pci.h>

#ifdef __cplusplus
extern "C" {
#endif

int piix4_isa_pci_probe(PCIDevice* pci_dev);
int piix4_ide_pci_probe(PCIDevice* pci_dev);

#ifdef __cplusplus
}
#endif

#endif //GZOS_PIXX4_ISA_H
