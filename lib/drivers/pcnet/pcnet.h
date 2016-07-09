//
// Created by gz on 7/2/16.
//

#ifndef GZOS_PCNET_H
#define GZOS_PCNET_H

#include <platform/pci/pci.h>
#include <memory>

#ifdef __cplusplus

/* The PCNET Rx and Tx ring descriptors. */
struct PCnetRxDescriptor {
    uint32_t base;
    int16_t buf_length;
    int16_t status;
    uint32_t msg_length;
    uint32_t reserved;
};

struct PCnetTxDescriptor {
    uint32_t base;
    int16_t length;
    int16_t status;
    uint32_t misc;
    uint32_t reserved;
};

/* The PCNET 32-Bit initialization block, described in databook. */
struct PCnetInitializationBlock {
    uint16_t mode;
    uint8_t rlen;
    uint8_t tlen;
    uint8_t phys_addr[6];
    uint16_t reserved;
    uint32_t filter[2];
    /* Receive and transmit ring base, along with extra bits. */
    uint32_t rx_ring;
    uint32_t tx_ring;
    uint32_t reserved2;
};

class pcnet_drv
{
public:
    static constexpr int PacketSize = 1548;
    static constexpr int RxRingSize = 32;
    static constexpr int TxRingSize = 8;
    using RxRingBufferType = uint8_t[RxRingSize][PacketSize];
    struct RingBuffers {
        PCnetRxDescriptor rxRing[RxRingSize];
        PCnetTxDescriptor txRing[TxRingSize];
    };

public:
    pcnet_drv(PCIDevice* pci_dev);
    pcnet_drv(pcnet_drv&& other);

    bool initialize(void);
    bool start(void);

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

    bool identifyChip(void);
    bool readMac(void);
    bool setupBufferRings(void);
    bool setupInitializationBlock(void);
    void reset(void) const;
    bool check(void);

    // IO Read/Write
    uint8_t ioreg(Registers reg) const;
    uint32_t ioread(Registers reg) const;
    void iowrite(Registers reg, uint32_t value);

    // Registers ops
    uint32_t read_csr(uint32_t index);
    void write_csr(uint32_t index, uint32_t val);
    uint32_t read_bcr(uint32_t index);
    void write_bcr(uint32_t index, uint32_t val);

private:
    char _name[16];
    PCIDevice* _pcidev;
    uintptr_t _ioport;
    bool _dwio;
    const char* _chipName;
    uint8_t _mac[6];
    std::unique_ptr<RxRingBufferType> _rxBuffers;
    std::unique_ptr<RingBuffers> _ringBuffers;
    std::unique_ptr<PCnetInitializationBlock> _initBlock;
    int _currentRxBuf;
    int _currentTxBuf;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

int pcnet_pci_probe(PCIDevice* pci_dev);

#ifdef __cplusplus
}
#endif
#endif //GZOS_PCNET_H
