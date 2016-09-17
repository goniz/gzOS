#include <pcnet.h>
#include <platform/kprintf.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <platform/cpu.h>
#include <platform/clock.h>
#include <malloc.h>
#include <lib/primitives/align.h>
#include <lib/network/nbuf.h>
#include <lib/network/ethernet.h>
#include <platform/malta/clock.h>
#include "pcnet.h"

// BIG TODO: use nbufs in the rx + tx ring buffers to prevent copying data between the layers.. should be awesome!

DECLARE_PCI_DRIVER(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_LANCE, pcnet, pcnet_pci_probe);
static std::vector<pcnet_drv> __pcnet_devices;

static std::atomic<int> __dev_index(0);

static int generate_dev_index(void) {
    return __dev_index.fetch_add(1, std::memory_order::memory_order_relaxed);
}

extern "C"
int pcnet_pci_probe(PCIDevice *pci_dev) {
    __pcnet_devices.emplace_back(pci_dev);
    pcnet_drv& drv = __pcnet_devices.back();

    if (!drv.initialize()) {
        return 1;
    }

    if (!drv.start()) {
        return 1;
    }

    return 0;
}

pcnet_drv::pcnet_drv(PCIDevice *pci_dev)
        : _pcidev(pci_dev),
          _ioport(pci_dev ? (uintptr_t) pci_dev->iomem(0) : 0),
          _irq(pci_dev ? pci_dev->irq() : -1),
          _dwio(false),
          _chipName(nullptr),
          _mac{},
          _rxBuffers(nullptr),
          _ringBuffers(nullptr),
          _initBlock(nullptr),
          _currentTxBuf(0) {
    // conditionally name the device to allow ctor reuse..
    if (_pcidev) {
        sprintf(_name, "pcnet%d", generate_dev_index());
    }

    memset(&_counters, 0, sizeof(_counters));
}

pcnet_drv::pcnet_drv(pcnet_drv &&other)
        : pcnet_drv(nullptr) {
    std::swap(_pcidev, other._pcidev);
    std::swap(_ioport, other._ioport);
    std::swap(_dwio, other._dwio);
    std::swap(_chipName, other._chipName);
    std::swap(_rxBuffers, other._rxBuffers);
    std::swap(_ringBuffers, other._ringBuffers);
    std::swap(_initBlock, other._initBlock);
    std::swap(_currentTxBuf, other._currentTxBuf);

    strncpy(_name, other._name, 16);
    memset(other._name, 0, sizeof(other._name));

    memcpy(_mac, other._mac, sizeof(_mac));
    memset(other._mac, 0, sizeof(other._mac));
}

bool pcnet_drv::initialize(void) {
    uint16_t config_register = _pcidev->readHalf(PCI_COMMAND);
    config_register |= PCI_COMMAND_IO;    // enable IO access
    config_register |= PCI_COMMAND_MASTER;// enable master access
    _pcidev->writeHalf(PCI_COMMAND, config_register);

    uint16_t new_config_reg = _pcidev->readHalf(PCI_COMMAND);
    if ((new_config_reg & config_register) != config_register) {
        kprintf("%s: Could not enable IO access or Bus Mastering.. got %04x expected %04x\n", _name, new_config_reg,
                config_register);
        return false;
    }

    _pcidev->writeByte(PCI_LATENCY_TIMER, 0x40);

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

    if (!this->acquireIrq()) {
        return false;
    }

    return (bool) ethernet_register_device(_name, _mac, this, pcnet_drv::xmit);
}

bool pcnet_drv::start(void) {
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

    val = this->read_bcr(9) & ~3;
    val |= 1;
    this->write_bcr(9, val);

    val = this->read_bcr(32);
    val |= 0x80;
    this->write_bcr(32, val);

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
    val |=  (1 << 9);    // set tx int mask - disable tx int
    val |=  (1 << 8);    // set init int mask - disable init int
    val &= ~(1 << 2);   // clear big endian flag
    this->write_csr(3, val);

    platform_enable_irq(_irq);

    /*
    * Tell the controller where the Init Block is located.
    */
    uintptr_t addr = platform_iomem_virt_to_phy((uintptr_t) _initBlock.get());
    this->write_csr(1, addr & 0xffff);
    this->write_csr(2, (addr >> 16) & 0xffff);

    this->write_csr(4, 0x0915);
    this->write_csr(0, CSR0_INIT);    /* start */

    /* Wait for Init Done bit */
    int i;
    for (i = 10000; i > 0; i--) {
        if (this->read_csr(0) & CSR0_IDON) {
            break;
        }

        clock_delay_ms(1);
    }

    if (i <= 0) {
        kprintf("%s: TIMEOUT: controller init failed\n", _name);
        this->reset();
        _dwio = false;
        return false;
    }

    /*
     * Finally start network controller operation.
     */
    this->write_csr(0, CSR0_NORMAL | CSR0_INTEN);
    return true;
}

bool pcnet_drv::identifyChip(void) {
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

bool pcnet_drv::readMac(void) {
    /*
     * In most chips, after a chip reset, the ethernet address is read from
     * the station address PROM at the buf address and programmed into the
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

bool pcnet_drv::setupBufferRings(void) {
    // allocate RX buffers, pcnet likes 16 byte alignment for things..
    void *rxBuffers = memalign(pcnet_drv::PacketSize * pcnet_drv::RxRingSize, 16);
    if (nullptr == rxBuffers) {
        return false;
    }

    flush_dcache_range((uintptr_t) rxBuffers,
                       (uintptr_t) ((uintptr_t) rxBuffers + (pcnet_drv::PacketSize * pcnet_drv::RxRingSize)));
    _rxBuffers = std::unique_ptr<RxRingBufferType>(
            (RxRingBufferType *) platform_buffered_virt_to_unbuffered_virt((uintptr_t) rxBuffers)
    );

    // allocate RX/TX ring buffers
    void *ringBuffers = memalign(sizeof(RingBuffers), 16);
    if (nullptr == ringBuffers) {
        return false;
    }

    flush_dcache_range((uintptr_t) ringBuffers, (uintptr_t) ((uintptr_t) ringBuffers + sizeof(RingBuffers)));
    _ringBuffers = std::unique_ptr<RingBuffers>(
            (RingBuffers *) platform_buffered_virt_to_unbuffered_virt((uintptr_t) ringBuffers)
    );

    for (int i = 0; i < RxRingSize; i++) {
        PCnetRxDescriptor *rd = &(_ringBuffers->rxRing[i]);
        RxRingBufferType *buf = (RxRingBufferType *) (_rxBuffers.get() + i);
        rd->buf = cpu_to_le32(platform_iomem_virt_to_phy((uintptr_t) buf));
        rd->buf_size = cpu_to_le16(-PacketSize);
        rd->status = cpu_to_le16(0x8000);
        rd->reserved = 0;
    }

    _currentTxBuf = 0;
    for (int i = 0; i < TxRingSize; i++) {
        PCnetTxDescriptor *rd = &(_ringBuffers->txRing[i]);
        rd->buf = 0;
        rd->buf_size = 0;
        rd->status = 0;
    }

    return true;
}

bool pcnet_drv::setupInitializationBlock(void) {
    void *initBlock = memalign(sizeof(PCnetInitializationBlock), 4);
    if (nullptr == initBlock) {
        return false;
    }

    flush_dcache_range((uintptr_t) initBlock, (uintptr_t) ((uintptr_t) initBlock + sizeof(PCnetInitializationBlock)));
    _initBlock = std::unique_ptr<PCnetInitializationBlock>(
            (PCnetInitializationBlock *) platform_buffered_virt_to_unbuffered_virt((uintptr_t) initBlock)
    );

    _initBlock->mode = 0x0000; //cpu_to_le16(0x0003);
    _initBlock->filter[0] = 0x00000000;
    _initBlock->filter[1] = 0x00000000;

    /*
    * Setup Init Block.
    */
    memcpy(_initBlock->phys_addr, _mac, sizeof(_mac));

    _initBlock->rlen = log2(RxRingSize) << 4;
    _initBlock->tlen = log2(TxRingSize) << 4;
    _initBlock->rx_ring = cpu_to_le32(platform_iomem_virt_to_phy((uintptr_t)_ringBuffers->rxRing));
    _initBlock->tx_ring = cpu_to_le32(platform_iomem_virt_to_phy((uintptr_t)_ringBuffers->txRing));

    return true;
}

uint8_t pcnet_drv::ioreg(Registers reg) const {
    const int multiplier = _dwio ? 4 : 2;
    return (uint8_t) (RegistersBase + ((int) reg * multiplier));
}

uint32_t pcnet_drv::ioread(Registers reg) const {
    const uint8_t offset = this->ioreg(reg);
    if (_dwio) {
        return le32_to_cpu(*((volatile uint32_t *) (_ioport + offset)));
    } else {
        return le16_to_cpu(*((volatile uint16_t *) (_ioport + offset)));
    }
}

void pcnet_drv::iowrite(Registers reg, uint32_t value) {
    const uint8_t offset = this->ioreg(reg);
    if (_dwio) {
        *((volatile uint32_t *) (_ioport + offset)) = cpu_to_le32(value);
    } else {
        *((volatile uint16_t *) (_ioport + offset)) = cpu_to_le16(value & 0xFFFF);
    }
}

uint32_t pcnet_drv::read_csr(uint32_t index) {
    this->iowrite(Registers::RAP, index);
    return this->ioread(Registers::RDP);
}

void pcnet_drv::write_csr(uint32_t index, uint32_t val) {
    this->iowrite(Registers::RAP, index);
    this->iowrite(Registers::RDP, val);
}

uint32_t pcnet_drv::read_bcr(uint32_t index) {
    this->iowrite(Registers::RAP, index);
    return this->ioread(Registers::BDP);
}

void pcnet_drv::write_bcr(uint32_t index, uint32_t val) {
    this->iowrite(Registers::RAP, index);
    this->iowrite(Registers::BDP, val);
}

void pcnet_drv::reset(void) const {
    this->ioread(Registers::RST);
}

bool pcnet_drv::check(void) {
    this->iowrite(Registers::RAP, 88);
    return this->ioread(Registers::RAP) == 88;
}

bool pcnet_drv::sendPacket(const void *packet, uint16_t length) {
    int i = 0;
    bool ret = true;
    short status = 0;
    struct PCnetTxDescriptor *txd = &_ringBuffers->txRing[_currentTxBuf];

    /* Wait for completion by testing the OWN bit */
    for (i = 1000; i > 0; i--) {
        status = le16_to_cpu(txd->status);
        if ((status & 0x8000) == 0) {
            break;
        }

        platform_cpu_wait();
    }

    if (i <= 0) {
        printf("%s: TIMEOUT: Tx%d failed (status = 0x%04x)\n",
               _name, _currentTxBuf, status);
        ret = false;
        goto exit;
    }

    /*
     * Setup Tx ring. Caution: the write order is important here,
     * set the status with the "ownership" bits last.
     */
    flush_dcache_range((uintptr_t) packet, (uintptr_t) packet + length);
    txd->buf_size = cpu_to_le16((int16_t) -length);
    txd->misc = 0;
    txd->buf = cpu_to_le32(platform_virt_to_phy((uintptr_t) packet));
    txd->status = cpu_to_le16((int16_t) 0x8300);

    /* Trigger an immediate send poll. */
    this->write_csr(CSR0, this->read_csr(CSR0) | 0x0008);

exit:
//    /* Wait for completion by testing the OWN bit */
//    for (i = 1000; i > 0; i--) {
//        status = le16_to_cpu(txd->status);
//        if ((status & 0x8000) == 0) {
//            break;
//        }
//
//        platform_cpu_wait();
//    }

    if (++_currentTxBuf >= TxRingSize) {
        _currentTxBuf = 0;
    }

    return ret;
}

void pcnet_drv::drainRxRing(void)
{
    uint16_t pkt_len = 0;
    void* buf = nullptr;

    for (int i = 0; i < RxRingSize; i++) {
        struct PCnetRxDescriptor* rdx = &_ringBuffers->rxRing[i];

        /*
         * If we own the next entry, it's a new packet. Send it up.
         */
        int16_t status = le16_to_cpu(rdx->status);
        if ((status & 0x8000) != 0) {
            continue;
        }

        status >>= 8;

        if (status != 0x03) {   /* There was an error. */
            /*
             * There is a tricky error noted by John Murphy,
             * <murf@perftech.com> to Russ Nelson: Even with full-sized
             * buffers it's possible for a jabber packet to use two
             * buffers, with only the last correctly noting the error.
             */
            if (status & 0x01)  /* Only count a general error at the */
                _counters.rx_errors++; /* end of a packet. */
            if (status & 0x20)
                _counters.rx_frame_errors++;
            if (status & 0x10)
                _counters.rx_over_errors++;
            if (status & 0x08)
                _counters.rx_crc_errors++;
            if (status & 0x04)
                _counters.rx_fifo_errors++;


        } else {
            pkt_len = (le32_to_cpu(rdx->msg_length) & 0xfff) - 4;

            /* Discard oversize frames. */
            if ((pkt_len > PacketSize) || (pkt_len < 60)) {
                _counters.rx_errors++;
            } else {
                buf = (void*)platform_iomem_phy_to_virt(le32_to_cpu(rdx->buf));
                if (-1 == (int) buf) {
                    goto release_rdx;
                }

                rdx->buf = cpu_to_le32(platform_iomem_virt_to_phy((uintptr_t) buf));
                invalidate_dcache_range((unsigned long)buf,
                                        (unsigned long)buf + pkt_len);

                NetworkBuffer* packetBuffer = nbuf_alloc(pkt_len);
                if (!nbuf_is_valid(packetBuffer)) {
                    kprintf("%s: packet pool exhausted. dropping frame..\n", _name);
                } else {
                    memcpy(nbuf_data(packetBuffer), buf, pkt_len);
                    nbuf_set_size(packetBuffer, pkt_len);

                    ethernet_absorb_packet(packetBuffer, _name);
                    nbuf_free(packetBuffer);
                }
            }
        }

    release_rdx:
        RxRingBufferType* origbuf = (RxRingBufferType *) (_rxBuffers.get() + i);
        rdx->buf = cpu_to_le32(platform_iomem_virt_to_phy((uintptr_t) origbuf));
        rdx->buf_size = cpu_to_le16(-PacketSize);
        rdx->reserved = 0;
        rdx->status = cpu_to_le16(0x8000);
    }
}

bool pcnet_drv::acquireIrq(void)
{
    _pcidev->writeByte(PCI_INTERRUPT_LINE, (uint8_t) _irq);

    return (bool) platform_register_irq(_irq, "pcnet_drv", (irq_handler_t) this->irqHandler, this);
}

void pcnet_drv::irqHandler(struct user_regs *regs, void *data)
{
    pcnet_drv* self = (pcnet_drv *) data;
    uint16_t csr0;
    int count = 100;

    csr0 = (uint16_t) self->read_csr(CSR0);
    while (csr0 & 0x8f00 && count > 0) {
        if (csr0 == 0xffff) {
            break;  /* PCMCIA remove happened */
        }

        /* Acknowledge all of the current interrupt sources ASAP. */
        self->write_csr(CSR0, csr0 & ~0x004fUL);

        /* Log misc errors. */
        if (csr0 & 0x4000) {
            /* Tx babble. */
            self->_counters.tx_errors++;
        }

        if (csr0 & 0x1000) {
            /*
             * This happens when our receive ring is full. This
             * shouldn't be a problem as we will see normal rx
             * interrupts for the frames in the receive ring.  But
             * there are some PCI chipsets (I can reproduce this
             * on SP3G with Intel saturn chipset) which have
             * sometimes problems and will fill up the receive
             * ring with error descriptors.  In this situation we
             * don't get a rx interrupt, but a missed frame
             * interrupt sooner or later.
             */
            self->_counters.rx_errors++;
        }

        if (csr0 & 0x0800) {
            /* unlike for the lance, there is no restart needed */
        }

        self->drainRxRing();

        csr0 = (uint16_t) self->read_csr(CSR0);
        count--;
    }

    csr0 |= CSR0_INTEN;
    self->write_csr(CSR0, csr0);
}

int pcnet_drv::xmit(void *user_ctx, NetworkBuffer* packetBuffer) {
    pcnet_drv* self = (pcnet_drv *) user_ctx;

    return self->sendPacket(nbuf_data(packetBuffer), (uint16_t) nbuf_size(packetBuffer));
}
