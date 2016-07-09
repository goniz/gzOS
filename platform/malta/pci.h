#ifndef __MALTA_PCI_H__
#define __MALTA_PCI_H__


#include <platform/pci/pci.h>

uint32_t platform_pci_bus_read_word(int dev, int devfn, int reg);
void platform_pci_bus_write_word(int dev, int devfn, int reg, uint32_t value);
intptr_t platform_pci_memory_base(void);
intptr_t platform_pci_io_base(void);

#endif /* _PCI_H_ */
