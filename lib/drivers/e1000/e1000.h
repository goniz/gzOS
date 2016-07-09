//
// Created by gz on 7/2/16.
//

#ifndef GZOS_PCNET_H
#define GZOS_PCNET_H

#include <platform/pci/pci.h>

#ifdef __cplusplus

class e1000_drv
{
public:
    e1000_drv(PCIDevice* pci_dev);
    e1000_drv(e1000_drv&& other);

    bool initialize(void);

    const char* name(void) const {
        return _name;
    }

private:
    /* Offsets from base I/O address for WIO mode */
    static constexpr int RegistersBase = 0x10;
    enum class Registers {
        RDP = 0,
        RAP = 1,
        RST = 2,
        BDP = 3
    };

    uint8_t ioreg(Registers reg) const;
    uint32_t ioread(Registers reg) const;
    void iowrite(Registers reg, uint32_t value);

    uint32_t read_csr(uint32_t index);
    void write_csr(uint32_t index, uint32_t val);
    uint32_t read_bcr(uint32_t index);
    void write_bcr(uint32_t index, uint32_t val);
    void reset(void) const;
    bool check(void);

private:
    char _name[16];
    PCIDevice* _pcidev;
    uintptr_t _iobase;
    bool _dwio;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

int e1000_pci_probe(PCIDevice* pci_dev);

#ifdef __cplusplus
}
#endif
#endif //GZOS_PCNET_H
