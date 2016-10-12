//
// Created by gz on 7/2/16.
//

#include <e1000.h>
#include <platform/kprintf.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <vector>
#include <platform/cpu.h>
#include <platform/clock.h>
#include <algorithm>

DECLARE_PCI_DRIVER(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82540EM, e1000, e1000_pci_probe);
static std::vector<e1000_drv> __e1000_devices;

static std::atomic<int> __dev_index(0);
static int generate_dev_index(void)
{
    return __dev_index.fetch_add(1, std::memory_order::memory_order_relaxed);
}

extern "C"
int e1000_pci_probe(PCIDevice* pci_dev)
{
    e1000_drv drv(pci_dev);

    kprintf("dev name: %s\n", drv.name());
    kprintf("pci bus %d dev %d devfn %d\n",
            pci_dev->bus(),
            pci_dev->dev(),
            pci_dev->devfunc());

    if (!drv.initialize()) {
        return 1;
    }

    __e1000_devices.push_back(std::move(drv));
    return 0;
}

e1000_drv::e1000_drv(PCIDevice *pci_dev)
    : _pcidev(pci_dev),
      _iobase(platform_iomem_phy_to_virt((uintptr_t) pci_dev->iomem(0))),
      _dwio(false)
{
    sprintf(_name, "e1000#%d", generate_dev_index());

    kprintf("%s: iobase %08x\n", _name, _iobase);
}

e1000_drv::e1000_drv(e1000_drv &&other)
    : _pcidev(nullptr),
      _iobase(0),
      _dwio(false)
{
    std::swap(_pcidev, other._pcidev);
    std::swap(_iobase, other._iobase);
    std::swap(_dwio, other._dwio);
    strncpy(_name, other._name, 16);
    memset(other._name, 0, sizeof(other._name));
}

uint8_t e1000_drv::ioreg(Registers reg) const
{
    const int multiplier = _dwio ? 4 : 2;
    return (uint8_t) (RegistersBase + ((int)reg * multiplier));
}

uint32_t e1000_drv::ioread(Registers reg) const
{
    const uint8_t offset = this->ioreg(reg);
//    kprintf("%s: ioread(%d) offset 0x%02x\n", _name, reg, offset);
    if (_dwio) {
        return *((volatile uint32_t*)(_iobase + offset));
    } else {
        return *((volatile uint16_t*)(_iobase + offset));
    }
}

void e1000_drv::iowrite(Registers reg, uint32_t value)
{
    const uint8_t offset = this->ioreg(reg);
//    kprintf("%s: iowrite(%d) offset 0x%02x value %08x\n", _name, reg, offset, value);
    if (_dwio) {
        *((volatile uint32_t*)(_iobase + offset)) = (value);
    } else {
        *((volatile uint16_t*)(_iobase + offset)) = (uint16_t)(value & 0xFFFF);
    }
}

uint32_t e1000_drv::read_csr(uint32_t index)
{
    this->iowrite(Registers::RAP, index);
    return this->ioread(Registers::RDP);
}

void e1000_drv::write_csr(uint32_t index, uint32_t val)
{
    this->iowrite(Registers::RAP, index);
    this->iowrite(Registers::RDP, val);
}

uint32_t e1000_drv::read_bcr(uint32_t index)
{
    this->iowrite(Registers::RAP, index);
    return this->ioread(Registers::BDP);
}

void e1000_drv::write_bcr(uint32_t index, uint32_t val)
{
    this->iowrite(Registers::RAP, index);
    this->iowrite(Registers::BDP, val);
}

void e1000_drv::reset(void) const
{
    this->ioread(Registers::RST);
}

bool e1000_drv::check(void)
{
    this->iowrite(Registers::RAP, 88);
    return this->ioread(Registers::RAP) == 88;
}

bool e1000_drv::initialize(void)
{
    kprintf("%s: PCI_BASE_ADDRESS_0 = %08x\n", _name, _pcidev->readWord(PCI_BASE_ADDRESS_0));
//    _pcidev->writeWord(PCI_BASE_ADDRESS_0, 0xffffffff);
    kprintf("%s: PCI_BASE_ADDRESS_0 = %08x\n", _name, _pcidev->readWord(PCI_BASE_ADDRESS_0));

    uint16_t config_register = le16_to_cpu(_pcidev->readHalf(PCI_COMMAND));

//    config_register &= 0xffff0000;        // preserve new_config_reg register, clear config register
//    config_register |= PCI_COMMAND_IO;    // enable IO access
    config_register |= PCI_COMMAND_MEMORY;// enable memory access
    config_register |= PCI_COMMAND_MASTER;// enable master access

    _pcidev->writeHalf(PCI_COMMAND, (config_register));

    uint32_t new_config_reg = _pcidev->readHalf(PCI_COMMAND);
    if ((new_config_reg & config_register) != config_register) {
        kprintf("%s: Could not enable IO access or Bus Mastering.. got %08x expected %08x\n", _name, new_config_reg, config_register);
        return false;
    }

    return true;
}




