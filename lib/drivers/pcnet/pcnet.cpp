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
#include <malloc.h>
#include <cassert>
#include <lib/primitives/align.h>
#include <complex>
#include <platform/interrupts.h>

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

    if (!drv.initialize()) {
        return 1;
    }

    if (!drv.start()) {
        return 1;
    }

    __pcnet_devices.push_back(std::move(drv));
    return 0;
}

extern "C"
struct user_regs* pcnet_handler(struct user_regs* regs)
{
    return regs;
}

pcnet_drv::pcnet_drv(PCIDevice *pci_dev)
    : _pcidev(pci_dev),
      _ioport(pci_dev ? (uintptr_t) pci_dev->iomem(0) : 0),
      _dwio(false),
      _chipName(nullptr),
      _mac{},
      _rxBuffers(nullptr),
      _ringBuffers(nullptr),
      _initBlock(nullptr),
      _currentRxBuf(0),
      _currentTxBuf(0)
{
    // conditionally name the device to allow ctor reuse..
    if (_pcidev) {
        sprintf(_name, "pcnet%d", generate_dev_index());
    }
}

pcnet_drv::pcnet_drv(pcnet_drv &&other)
    : pcnet_drv(nullptr)
{
    std::swap(_pcidev, other._pcidev);
    std::swap(_ioport, other._ioport);
    std::swap(_dwio, other._dwio);
    std::swap(_chipName, other._chipName);
    std::swap(_rxBuffers, other._rxBuffers);
    std::swap(_ringBuffers, other._ringBuffers);
    std::swap(_initBlock, other._initBlock);
    std::swap(_currentRxBuf, other._currentRxBuf);
    std::swap(_currentTxBuf, other._currentTxBuf);

    strncpy(_name, other._name, 16);
    memset(other._name, 0, sizeof(other._name));

    memcpy(_mac, other._mac, sizeof(_mac));
    memset(other._mac, 0, sizeof(other._mac));
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

    // reset the device
    _dwio = true;
    this->reset();
    _dwio = false;
    this->reset();

    clock_delay_ms(1);

    // check if register access is working
    if (this->read_csr(0) != 4 || !this->check()) {
        kprintf("%s: CSR register access check failed\n", this->name());
        return false;
    }

    if (!this->identifyChip()) {
        return false;
    }

    if (!this->readMac()) {
        return false;
    }

    kprintf("%s: MAC Address %02x:%02x:%02x:%02x:%02x:%02x\n", _name,
            _mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]);

    if (!this->setupBufferRings()) {
        return false;
    }

    if (!this->setupInitializationBlock()) {
        return false;
    }

    return true;
}

bool pcnet_drv::start(void)
{
    // turn on 32-bit mode
    this->write_bcr(20, 2);
    _dwio = true;

    /* Set/reset autoselect bit */
    uint32_t val = this->read_bcr(2) & ~2;
    val |= 2;
    this->write_bcr(2, val);

    /* Enable auto negotiate, setup, disable fd */
    val = this->read_bcr(32) & ~0x98;
    val |= 0x20;
    this->write_bcr(32, val);

    /*
    * Enable NOUFLO on supported controllers, with the transmit
    * start point set to the full packet. This will cause entire
    * packets to be buffered by the ethernet controller before
    * transmission, eliminating underflows which are common on
    * slower devices. Controllers which do not support NOUFLO will
    * simply be left with a larger transmit FIFO threshold.
    */
    val = this->read_bcr(18);
    val |= 1 << 11;
    this->write_bcr(18, val);

    val = this->read_csr(80);
    val |= 0x3 << 10;
    this->write_csr(80, val);

    // setup interrupts masks
    val = this->read_csr(3);
    val &= ~(1 << 10);  // clear rx int mask - enable rx int
    val |= (1 << 9);    // set tx int mask - disable tx int
    val |= (1 << 8);    // set init int mask - disable init int
    val &= ~(1 << 2);   // clear big endian flag
    this->write_csr(3, val);

    /*
    * Tell the controller where the Init Block is located.
    */
    uintptr_t addr = platform_iomem_virt_to_phy((uintptr_t) _initBlock.get());
    this->write_csr(1, addr & 0xffff);
    this->write_csr(2, (addr >> 16) & 0xffff);

    this->write_csr(4, 0x0915);
    this->write_csr(0, 0x0001);    /* start */

    /* Wait for Init Done bit */
    int i;
    for (i = 10000; i > 0; i--) {
        if (this->read_csr(0) & 0x0100) {
            break;
        }
        clock_delay_ms(1);
    }

    if (i <= 0) {
        kprintf("%s: TIMEOUT: controller init failed\n", _name);
        this->reset();
        return false;
    }

    /*
     * Finally start network controller operation.
     */
    this->write_csr(0, 0x0002);

//    _pcidev->writeByte(PCI_INTERRUPT_LINE, 4);
//    uint32_t isrMask = interrupts_disable();
//    isrMask |= SR_IM0;
//    isrMask |= SR_IM1;
//    isrMask |= SR_IM3;
//    isrMask |= SR_IM4;
//    isrMask |= SR_IM5;
//    isrMask |= SR_IM6;
//    interrupts_enable(isrMask);
    return true;
}

bool pcnet_drv::identifyChip(void)
{
    /* Identify the chip */
    uint32_t chip_version = (this->read_csr(88) | (this->read_csr(89) << 16));
    if ((chip_version & 0xfff) != 0x003) {
        return false;
    }

    chip_version = (uint16_t) ((chip_version >> 12) & 0xffff);
    switch (chip_version) {
        case 0x2621:
            _chipName = "AMD PCnet/PCI II 79C970A";  /* PCI */
            break;
        case 0x2625:
            _chipName = "AMD PCnet/FAST III 79C973"; /* PCI */
            break;
        case 0x2627:
            _chipName = "AMD PCnet/FAST III 79C975"; /* PCI */
            break;
        default:
            printf("%s: PCnet version %lx not supported\n", _name, chip_version);
            return -1;
    }

    return true;
}

bool pcnet_drv::readMac(void)
{
    /*
     * In most chips, after a chip reset, the ethernet address is read from
     * the station address PROM at the base address and programmed into the
     * "Physical Address Registers" CSR12-14.
     */
    for (int i = 0; i < 3; i++) {
        uint32_t val = this->read_csr((uint32_t) (i + 12)) & 0x0ffff;
        /* There may be endianness issues here. */
        _mac[2 * i] = (uint8_t) (val & 0x0ff);
        _mac[2 * i + 1] = (uint8_t) ((val >> 8) & 0x0ff);
    }

    return true;
}

bool pcnet_drv::setupBufferRings(void)
{
    // allocate RX buffers, pcnet likes 16 byte alignment for things..
    void* rxBuffers = memalign(pcnet_drv::PacketSize * pcnet_drv::RxRingSize, 16);
    if (nullptr == rxBuffers) {
        return false;
    }

    _rxBuffers = std::unique_ptr<RxRingBufferType>(
            (RxRingBufferType*) platform_buffered_virt_to_unbuffered_virt((uintptr_t)rxBuffers)
    );

    // allocate RX/TX ring buffers
    void* ringBuffers = memalign(sizeof(RingBuffers), 16);
    if (nullptr == ringBuffers) {
        return false;
    }

    _ringBuffers = std::unique_ptr<RingBuffers>(
            (RingBuffers*) platform_buffered_virt_to_unbuffered_virt((uintptr_t)ringBuffers)
    );

    _currentRxBuf = 0;
    for (int i = 0; i < RxRingSize; i++) {
        PCnetRxDescriptor* rd = &(_ringBuffers->rxRing[i]);
        RxRingBufferType* buf = (RxRingBufferType *) (_rxBuffers.get() + i);
        rd->base = cpu_to_le32(platform_iomem_virt_to_phy((uintptr_t)buf));
        rd->buf_length = cpu_to_le16(PacketSize);
        rd->status = cpu_to_le16(0x8000);
    }

    _currentTxBuf = 0;
    for (int i = 0; i < TxRingSize; i++) {
        PCnetTxDescriptor* rd = &(_ringBuffers->txRing[i]);
        rd->base = 0;
        rd->length = 0;
        rd->status = 0;
    }

    return true;
}

bool pcnet_drv::setupInitializationBlock(void)
{
    void* initBlock = memalign(sizeof(PCnetInitializationBlock), 4);
    if (nullptr == initBlock) {
        return false;
    }

    _initBlock = std::unique_ptr<PCnetInitializationBlock>(
            (PCnetInitializationBlock*) platform_buffered_virt_to_unbuffered_virt((uintptr_t)initBlock)
    );

    _initBlock->mode = cpu_to_le16(0x0000);
    _initBlock->filter[0] = 0x00000000;
    _initBlock->filter[1] = 0x00000000;

    /*
    * Setup Init Block.
    */
    for (int i = 0; i < 6; i++) {
        _initBlock->phys_addr[i] = _mac[i];
    }

    _initBlock->rlen = log2(RxRingSize) << 4;
    _initBlock->tlen = log2(TxRingSize) << 4;
    _initBlock->rx_ring = cpu_to_le32(platform_iomem_virt_to_phy((uintptr_t)_ringBuffers->rxRing));
    _initBlock->tx_ring = cpu_to_le32(platform_iomem_virt_to_phy((uintptr_t)_ringBuffers->txRing));

    return true;
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