#ifndef GZOS_VIRTIO_SHMEM_H
#define GZOS_VIRTIO_SHMEM_H

#include <platform/pci/pci.h>

#ifdef __cplusplus

struct VirtIOSharedMemoryRegs {
    uint32_t IntMask;
    uint32_t IntStatus;
    uint32_t IvPosition;
    uint32_t DoorBell;
};

class virtio_shmem_drv
{
public:
    virtio_shmem_drv(PCIDevice* pci_dev);
    virtio_shmem_drv(virtio_shmem_drv&& other);

    bool initialize(void);

private:
    static void irqHandler(struct user_regs *regs, void *data);

    PCIDevice* _pcidev;
    volatile struct VirtIOSharedMemoryRegs* _regs;
    volatile void* _shared_memory_region;
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
