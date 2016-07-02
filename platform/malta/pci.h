#ifndef __MALTA_PCI_H__
#define __MALTA_PCI_H__


#include <platform/pci/pci.h>

void platform_pci_bus_enumerate(pci_bus_t *pcibus);
intptr_t platform_pci_memory_base(void);
intptr_t platform_pci_io_base(void);

#endif /* _PCI_H_ */
