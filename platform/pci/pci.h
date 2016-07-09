//
// Created by gz on 7/2/16.
//
/* Please read http://wiki.osdev.org/PCI */

#ifndef GZOS_PCI_H
#define GZOS_PCI_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#include <vector>
#endif

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

#ifdef __cplusplus

struct PCIBar
{
    enum class MemoryType {
        IO,
        MEM_PREFETCH,
        MEM_NON_PREFETCH
    };

    uintptr_t phy_addr;
    size_t size;
    MemoryType type;
};

class PCIDevice
{
public:
    PCIDevice(int bus, int dev, int function);

    void assign_memory_region(uintptr_t* pci_mem_pos, uintptr_t* pci_io_pos);

    uint32_t readWord(int reg) const;
    uint16_t readHalf(int reg) const;
    uint8_t readByte(int reg) const;
    void writeWord(int reg, uint32_t val);
    void writeHalf(int reg, uint16_t val);
    void writeByte(int reg, uint8_t val);
    intptr_t iomem(int idx);

    int bus(void) const {
        return _busnum;
    }

    int dev(void) const {
        return _devnum;
    }

    int devfunc(void) const {
        return _devfunc;
    }

    uint16_t vendorId(void) const {
        return _vendor_id;
    }

    uint16_t deviceId(void) const {
        return _device_id;
    }

    uint8_t classCode(void) const {
        return _class_code;
    }

    uint8_t pin(void) const {
        return _pin;
    }

    uint8_t irq(void) const {
        return _irq;
    }

    const std::vector<PCIBar>& bars(void) const {
        return _bars;
    }

private:
    int _busnum;
    int _devnum;
    int _devfunc;
    uint16_t _vendor_id;
    uint16_t _device_id;
    uint8_t _class_code;
    uint8_t _pin;
    uint8_t _irq;
    std::vector<PCIBar> _bars;
};

class PCIBus
{
public:
    PCIBus(intptr_t pci_mem_base, size_t pci_mem_size,
           intptr_t pci_io_base, size_t pci_io_size);

    void enumerate(void);
    void assign_memory_regions(void);
    void dump(void) const;

    const std::vector<PCIDevice>& devices(void) const {
        return _devices;
    }

private:
    std::vector<PCIDevice> _devices;
    uintptr_t _pci_memory_pos;
    uintptr_t _pci_io_pos;
};
#else
    struct PCIBar;
    struct PCIDevice;
    struct PCIBus;
#endif //__cplusplus

void platform_pci_init(void);
void platform_pci_driver_probe(void);

// bus functions
uint32_t platform_pci_bus_read_word(int dev, int devfn, int reg);
void platform_pci_bus_write_word(int dev, int devfn, int reg, uint32_t value);
uint16_t platform_pci_bus_read_half(int dev, int devfn, int reg);
void platform_pci_bus_write_half(int dev, int devfn, int reg, uint16_t value);
uint8_t platform_pci_bus_read_byte(int dev, int devfn, int reg);
void platform_pci_bus_write_byte(int dev, int devfn, int reg, uint8_t value);


intptr_t platform_pci_memory_base(void);
size_t platform_pci_memory_size(void);
intptr_t platform_pci_io_base(void);
size_t platform_pci_io_size(void);

//uintptr_t platform_pci_device_get_iobase(pci_device_t* pcidev);
//uintptr_t platform_pci_device_get_ioport(pci_device_t* pcidev);

#ifdef __cplusplus
}
#endif

#include <platform/pci/pci_drivers.h>
#include <platform/pci/pci_commands.h>
#include <platform/pci/pci_ids.h>
#endif //GZOS_PCI_H
