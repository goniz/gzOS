#ifndef GZOS_VIRTIO_SHMEM_H
#define GZOS_VIRTIO_SHMEM_H

#include <platform/pci/pci.h>

#ifdef __cplusplus

enum class VirtIOSharedMemoryRegs {
    IntMask = 0,
    IntStatus = 4,
    IvPosition = 8,
    DoorBell = 12
};

class virtio_shmem_drv
{
public:
    virtio_shmem_drv(PCIDevice* pci_dev);
    virtio_shmem_drv(virtio_shmem_drv&& other);

    bool initialize(void);

private:
    void writeReg(VirtIOSharedMemoryRegs reg, uint32_t value);
    uint32_t readReg(VirtIOSharedMemoryRegs reg);
    static void irqHandler(struct user_regs *regs, void *data);

    PCIDevice* _pcidev;
    volatile uint8_t* _regs;
    volatile uint8_t* _shared_memory_region;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

int virtio_shmem_pci_probe(PCIDevice* pci_dev);

#ifdef __cplusplus
}
#endif
#endif //GZOS_VIRTIO_SHMEM_H
