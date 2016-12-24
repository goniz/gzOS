#ifndef GZOS_VIRTIO_SHMEM_H
#define GZOS_VIRTIO_SHMEM_H

#include <platform/pci/pci.h>
#include <lib/kernel/vfs/FileDescriptor.h>

#ifdef __cplusplus

enum class QEMUSharedMemoryRegs {
    IntMask = 0,
    IntStatus = 4,
    IvPosition = 8,
    DoorBell = 12
};

class qemu_shmem_drv
{
public:
    qemu_shmem_drv(PCIDevice* pci_dev);
    qemu_shmem_drv(qemu_shmem_drv&& other);

    bool initialize(void);

private:
    void writeReg(QEMUSharedMemoryRegs reg, uint32_t value);
    uint32_t readReg(QEMUSharedMemoryRegs reg);
    static void irqHandler(struct user_regs *regs, void *data);

    PCIDevice* _pcidev;
    volatile uint8_t* _regs;
    volatile uint8_t* _shared_memory_region;
};

class ShmemFileDescriptor : public FileDescriptor {
public:
    virtual int read(void* buffer, size_t size) override;
    virtual int write(const void* buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual void close(void) override;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

int qemu_shmem_pci_probe(PCIDevice* pci_dev);

#ifdef __cplusplus
}
#endif
#endif //GZOS_VIRTIO_SHMEM_H
