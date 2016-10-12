#include <platform/interrupts.h>
#include "virtio_shmem.h"
#include <cstdio>
#include <platform/kprintf.h>
#include <platform/clock.h>

DECLARE_PCI_DRIVER(PCI_VENDOR_ID_REDHAT_QUMRANET, PCI_DEVICE_ID_VIRTIO_SHARED_MEMORY, virtio_shmem, virtio_shmem_pci_probe);
static std::vector<virtio_shmem_drv> __shmem_devices;

int virtio_shmem_pci_probe(PCIDevice* pci_dev)
{
    __shmem_devices.emplace_back(pci_dev);
    virtio_shmem_drv& drv = __shmem_devices.back();

    if (!drv.initialize()) {
        return 1;
    }

    return 0;
}

virtio_shmem_drv::virtio_shmem_drv(PCIDevice *pci_dev)
    : _pcidev(pci_dev),
      _regs(0),
      _shared_memory_region(0)
{

}

virtio_shmem_drv::virtio_shmem_drv(virtio_shmem_drv &&other)
    : _pcidev(0),
      _regs(0),
      _shared_memory_region(0)
{
    std::swap(_pcidev, other._pcidev);
    std::swap(_regs, other._regs);
    std::swap(_shared_memory_region, other._shared_memory_region);
}

bool virtio_shmem_drv::initialize(void)
{
    if (nullptr == _pcidev) {
        return false;
    }

    _regs = (volatile uint8_t*) _pcidev->iomem(0);
    _shared_memory_region = (volatile uint8_t*) _pcidev->iomem(1);
    if (nullptr == _regs || nullptr == _shared_memory_region) {
        return false;
    }

    if (!platform_register_irq(_pcidev->irq(), "virtio_shmem", (irq_handler_t)virtio_shmem_drv::irqHandler, this)) {
        printf("virtio_shmem: Failed to register irq\n");
        return false;
    }

    _pcidev->writeByte(PCI_INTERRUPT_LINE, _pcidev->irq());

    uint16_t command = _pcidev->readHalf(PCI_COMMAND);
    command |= PCI_COMMAND_MASTER;
    command |= PCI_COMMAND_MEMORY;
    _pcidev->writeHalf(PCI_COMMAND, command);

    this->writeReg(VirtIOSharedMemoryRegs::IntMask, ~0UL);
    platform_enable_irq(_pcidev->irq());
    return true;
}

void virtio_shmem_drv::irqHandler(struct user_regs *regs, void *data)
{
    kprintf("IN VIRTIO IRQ HANDLER!!\n");
    virtio_shmem_drv* self = (virtio_shmem_drv *) data;

    auto status = self->readReg(VirtIOSharedMemoryRegs::IntStatus);
    if (!status || 0xffffffff == status) {
        return;
    }

    kprintf("status: %08x\n", status);

    self->writeReg(VirtIOSharedMemoryRegs::IntMask, ~0UL);
}

void virtio_shmem_drv::writeReg(VirtIOSharedMemoryRegs reg, uint32_t value) {
    volatile uint32_t* ptr = (uint32_t *) (_regs + (int)reg);
    *ptr = value;
}

uint32_t virtio_shmem_drv::readReg(VirtIOSharedMemoryRegs reg) {
    volatile uint32_t* ptr = (uint32_t *) (_regs + (int)reg);
    return *ptr;
}
