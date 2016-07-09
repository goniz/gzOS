//
// Created by gz on 7/2/16.
//
#include <platform/pci/pci.h>
#include <platform/kprintf.h>
#include <platform/cpu.h>
#include <cstdio>

static const pci_device_id *pci_find_device(const pci_vendor_id *vendor, uint16_t device_id);
static const pci_vendor_id *pci_find_vendor(uint16_t vendor_id);

PCIDevice::PCIDevice(int bus, int dev, int function)
        : _busnum(bus),
          _devnum(dev),
          _devfunc(function)
{
    _vendor_id = this->readHalf(PCI_VENDOR_ID);
    _device_id = this->readHalf(PCI_DEVICE_ID);
    _class_code = this->readByte(PCI_CLASS_CODE);
    _pin = this->readByte(PCI_INTERRUPT_PIN);
    _irq = this->readByte(PCI_INTERRUPT_LINE);
}

void PCIDevice::assign_memory_region(uintptr_t* pci_mem_pos, uintptr_t* pci_io_pos)
{
    this->writeWord(PCI_COMMAND, 0);

    for (int barReg = PCI_BASE_ADDRESS_0; barReg <= PCI_BASE_ADDRESS_5; barReg += 4)
    {
        PCIBar::MemoryType type;
        uint32_t bar = this->readWord(barReg);
        this->writeWord(barReg, 0xffffffffUL);
        uint32_t size = this->readWord(barReg);

        if (0 == size || bar == size) {
            continue;
        }

        if (bar & PCI_BAR_IO)  {
            type = PCIBar::MemoryType::IO;
            size &= ~PCI_BAR_IO_MASK;
            size = (uint32_t) -size;
            bar = *pci_io_pos = ((*pci_io_pos - 1) | (size - 1)) + 1;
            *pci_io_pos += size;
        } else {
            type = (bar & PCI_BAR_PREFETCHABLE) ? PCIBar::MemoryType::MEM_PREFETCH : PCIBar::MemoryType::MEM_NON_PREFETCH;
            size &= ~PCI_BAR_MEMORY_MASK;
            size = (uint32_t) -size;
            bar = *pci_mem_pos = ((*pci_mem_pos - 1) | (size - 1)) + 1;
            *pci_mem_pos += size;
        }

        this->writeWord(barReg, bar);
        _bars.push_back({bar, size, type});
    }
}

uint32_t PCIDevice::readWord(int reg) const
{
    return platform_pci_bus_read_word(_devnum, _devfunc, reg);
}

uint16_t PCIDevice::readHalf(int reg) const
{
    return platform_pci_bus_read_half(_devnum, _devfunc, reg);
}

uint8_t PCIDevice::readByte(int reg) const
{
    return platform_pci_bus_read_byte(_devnum, _devfunc, reg);
}

void PCIDevice::writeWord(int reg, uint32_t val)
{
    platform_pci_bus_write_word(_devnum, _devfunc, reg, val);
}

void PCIDevice::writeHalf(int reg, uint16_t val)
{
    platform_pci_bus_write_half(_devnum, _devfunc, reg, val);
}

void PCIDevice::writeByte(int reg, uint8_t val)
{
    platform_pci_bus_write_byte(_devnum, _devfunc, reg, val);
}

PCIBus::PCIBus(intptr_t pci_mem_base, size_t pci_mem_size,
       intptr_t pci_io_base, size_t pci_io_size)
{
    // TOOD: check the size..
    _pci_memory_pos = (uintptr_t) pci_mem_base;
    _pci_io_pos = (uintptr_t) pci_io_base;
}

intptr_t PCIDevice::iomem(int idx)
{
    if (_bars.empty()) {
        return 0;
    }

    const auto& bar = _bars[idx];
    if (bar.type == PCIBar::MemoryType::IO) {
        return platform_ioport_to_virt(bar.phy_addr);
    } else {
        return platform_iomem_phy_to_virt(bar.phy_addr);
    }
}

void PCIBus::enumerate(void)
{
    for (int dev = 0; dev < 32; dev++)
    {
        for (int devfn = 0; devfn < 8; devfn++)
        {
            if (~0UL == platform_pci_bus_read_word(dev, devfn, 0)) {
                continue;
            }

            _devices.emplace_back(0, dev, devfn);
        }
    }
}

void PCIBus::assign_memory_regions(void)
{
    for (PCIDevice& pcidev : _devices)
    {
        pcidev.assign_memory_region(&_pci_memory_pos, &_pci_io_pos);

        /* Configure Cache Line Size Register */
        pcidev.writeByte(PCI_CACHE_LINE_SIZE, 0x08);

        /* Configure Latency Timer */
        pcidev.writeByte(PCI_LATENCY_TIMER, 0x80);

        /* Disable interrupt line, if device says it wants to use interrupts */
        uint8_t pin = pcidev.readByte(PCI_INTERRUPT_PIN);
        if (pin != 0) {
            pcidev.writeByte(PCI_INTERRUPT_LINE, PCI_INTERRUPT_LINE_DISABLE);
        }
    }
}

void PCIBus::dump(void) const
{
    kprintf("PCI Devices: %d\n", _devices.size());

    for (const PCIDevice& pcidev : _devices)
    {
        char devstr[32];

        sprintf(devstr, "[pci:%02x:%02x.%02x]",
                pcidev.bus(),
                pcidev.dev(),
                pcidev.devfunc());

        const pci_vendor_id *vendor = pci_find_vendor(pcidev.vendorId());
        const pci_device_id *device = pci_find_device(vendor, pcidev.deviceId());

        kprintf("%s %s", devstr, pci_class_code[pcidev.classCode()]);

        if (vendor)
            kprintf(" %s", vendor->name);
        else
            kprintf(" vendor:$%04x", pcidev.vendorId());

        if (device)
            kprintf(" %s\n", device->name);
        else
            kprintf(" device:$%04x\n", pcidev.deviceId());

        if (pcidev.pin())
            kprintf("%s Interrupt: pin %c routed to IRQ %d\n",
                    devstr, 'A' + pcidev.pin() - 1, pcidev.irq());

        for (const PCIBar& bar : pcidev.bars()) {
            uint32_t addr = (uint32_t) bar.phy_addr;
            size_t size = bar.size;
            const char* type = nullptr;

            if (bar.type == PCIBar::MemoryType::IO) {
                type = "I/O ports";
            } else {
                if (bar.type == PCIBar::MemoryType::MEM_PREFETCH)
                    type = "Memory (prefetchable)";
                else
                    type = "Memory (non-prefetchable)";
            }

            kprintf("%s Region: %s at %p [size=$%x]\n",
                    devstr, type, (void *) addr, (unsigned) size);
        }
    }
}


static PCIBus* _pci_bus = nullptr;

extern "C"
void platform_pci_init(void)
{
    intptr_t pci_mem_base = platform_pci_memory_base();
    size_t pci_mem_size = platform_pci_memory_size();
    intptr_t pci_io_base = platform_pci_io_base();
    size_t pci_io_size = platform_pci_io_size();

    _pci_bus = new PCIBus(pci_mem_base, pci_mem_size,
                          pci_io_base, pci_io_size);

    _pci_bus->enumerate();
    _pci_bus->assign_memory_regions();
    _pci_bus->dump();
}

extern const struct pci_driver_ent pci_drivers_table[];
extern const uint8_t __pci_drv_start;
extern const uint8_t __pci_drv_end;

extern "C"
void platform_pci_driver_probe()
{
    const size_t n_drivers = ((&__pci_drv_end - &__pci_drv_start) / sizeof(struct pci_driver_ent));

    for (const PCIDevice& dev : _pci_bus->devices())
    {
        for (unsigned int drv_index = 0; drv_index < n_drivers; drv_index++)
        {
            const struct pci_driver_ent *driver = &pci_drivers_table[drv_index];
            if (dev.vendorId() == driver->vendor_id &&
                dev.deviceId() == driver->device_id) {

                kprintf("[pci] Probing %s PCI driver\n", driver->driver_name);
                int ret = driver->probe_func((PCIDevice *) &dev);
                if (0 == ret) {
                    kprintf("[pci] PCI driver %s initialized successfully.\n", driver->driver_name);
                }
            }
        }
    }
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
