//
// Created by gz on 7/2/16.
//

#include <lib/drivers/pcnet/pcnet.h>
#include <platform/pci/pci.h>
#include <platform/kprintf.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <vector>
#include <platform/malta/mips.h>
#include <platform/malta/gt64120.h>
#include <platform/cpu.h>
#include <platform/malta/malta.h>
#include <platform/clock.h>

DECLARE_PCI_DRIVER(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_LANCE, pcnet, pcnet_pci_probe);
static std::vector<pcnet_drv> __pcnet_devices;

static std::atomic<int> __dev_index(0);
static int generate_dev_index(void)
{
    return __dev_index.fetch_add(1, std::memory_order::memory_order_relaxed);
}

extern "C"
int pcnet_pci_probe(PCIDevice* pci_dev)
{
    pcnet_drv drv(pci_dev);

    kprintf("dev name: %s\n", drv.name());
    kprintf("pci bus %d dev %d devfn %d iobase %x ioport %x\n",
            pci_dev->bus(),
            pci_dev->dev(),
            pci_dev->devfunc(),
            pci_dev->iomem(1),
            pci_dev->iomem(0));

    if (!drv.initialize()) {
        return 1;
    }

    __pcnet_devices.push_back(std::move(drv));
    return 0;
}

pcnet_drv::pcnet_drv(PCIDevice *pci_dev)
    : _pcidev(pci_dev),
      _ioport((uintptr_t) pci_dev->iomem(0)),
      _dwio(false)
{
    sprintf(_name, "pcnet%d", generate_dev_index());
}

pcnet_drv::pcnet_drv(pcnet_drv &&other)
    : _pcidev(nullptr),
      _ioport(0),
      _dwio(false)
{
    std::swap(_pcidev, other._pcidev);
    std::swap(_ioport, other._ioport);
    std::swap(_dwio, other._dwio);
    strncpy(_name, other._name, 16);
    memset(other._name, 0, sizeof(other._name));
}

uint8_t pcnet_drv::ioreg(Registers reg) const
{
    const int multiplier = _dwio ? 4 : 2;
    return (uint8_t) (RegistersBase + ((int)reg * multiplier));
}

uint32_t pcnet_drv::ioread(Registers reg) const
{
    const uint8_t offset = this->ioreg(reg);
    if (_dwio) {
        return le32_to_cpu(*((volatile uint32_t*)(_ioport + offset)));
    } else {
        return le16_to_cpu(*((volatile uint16_t*)(_ioport + offset)));
    }
}

void pcnet_drv::iowrite(Registers reg, uint32_t value)
{
    const uint8_t offset = this->ioreg(reg);
//    kprintf("%s: iowrite(%d) offset 0x%02x value %08x\n", _name, reg, offset, value);
    if (_dwio) {
        *((volatile uint32_t*)(_ioport + offset)) = cpu_to_le32(value);
    } else {
        *((volatile uint16_t*)(_ioport + offset)) = cpu_to_le16(value & 0xFFFF);
    }
}

uint32_t pcnet_drv::read_csr(uint32_t index)
{
    this->iowrite(Registers::RAP, index);
    return this->ioread(Registers::RDP);
}

void pcnet_drv::write_csr(uint32_t index, uint32_t val)
{
    this->iowrite(Registers::RAP, index);
    this->iowrite(Registers::RDP, val);
}

uint32_t pcnet_drv::read_bcr(uint32_t index)
{
    this->iowrite(Registers::RAP, index);
    return this->ioread(Registers::BDP);
}

void pcnet_drv::write_bcr(uint32_t index, uint32_t val)
{
    this->iowrite(Registers::RAP, index);
    this->iowrite(Registers::BDP, val);
}

void pcnet_drv::reset(void) const
{
    this->ioread(Registers::RST);
}

bool pcnet_drv::check(void)
{
    this->iowrite(Registers::RAP, 88);
    return this->ioread(Registers::RAP) == 88;
}

bool pcnet_drv::initialize(void)
{
    uint32_t config_register = _pcidev->readWord(PCI_COMMAND);

    config_register &= 0xffff0000;        // preserve new_config_reg register, clear config register
    config_register |= PCI_COMMAND_IO;    // enable IO access
    config_register |= PCI_COMMAND_MASTER;// enable master access

    _pcidev->writeWord(PCI_COMMAND, config_register);

    uint32_t new_config_reg = _pcidev->readWord(PCI_COMMAND);
    if ((new_config_reg & config_register) != config_register) {
        kprintf("%s: Could not enable IO access or Bus Mastering.. got %08x expected %08x\n", _name, new_config_reg, config_register);
        return false;
    }

    _pcidev->writeWord(PCI_LATENCY_TIMER, 0x40);

    kprintf("%s: PCI_DEVICE_ID = %08x\n", _name, _pcidev->readWord(PCI_DEVICE_ID));
    kprintf("%s: PCI_HEADER_TYPE = %08x\n", _name, _pcidev->readWord(PCI_HEADER_TYPE));
    kprintf("%s: PCI_BASE_ADDRESS_0 = %08x\n", _name, _pcidev->readWord(PCI_BASE_ADDRESS_0));
    kprintf("%s: PCI_BASE_ADDRESS_1 = %08x\n", _name, _pcidev->readWord(PCI_BASE_ADDRESS_1));
    kprintf("%s: PCI_BASE_ADDRESS_2 = %08x\n", _name, _pcidev->readWord(PCI_BASE_ADDRESS_2));
    kprintf("%s: PCI_BASE_ADDRESS_3 = %08x\n", _name, _pcidev->readWord(PCI_BASE_ADDRESS_3));

    // reset the device
    _dwio = true;
    this->reset();
    _dwio = false;
    this->reset();

    clock_delay_ms(1000);

    kprintf("%s: CSR idx 89: %04x, check: %s\n", this->name(), this->read_csr(89), this->check() ? "true" : "false");

    for (int i = 0; i < 30; i++) {
        kprintf("%s: idx %d data %04x\n", this->name(), i, this->read_csr(i));
    }

    // check if register access is working
    if (this->read_csr(0) != 4 || !this->check()) {
        kprintf("%s: CSR register access check failed\n", this->name());
        return false;
    }

    return true;
}




