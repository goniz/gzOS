#include <cstdio>
#include <algorithm>
#include <cstring>
#include <atomic>
#include <cassert>
#include <lib/primitives/basic_queue.h>
#include <platform/drivers.h>
#include <lib/syscall/syscall.h>
#include <lib/kernel/proc/Scheduler.h>
#include <platform/clock.h>
#include "lib/network/nbuf.h"
#include "lib/network/ethernet/ethernet.h"

struct EthernetHandler {
    uint16_t ether_type;
    void *user_ctx;
    ethernet_handler_t handler;
};

struct EthernetDevice {
    char name[16];
    const char *phy;
    MacAddress mac;
    void *user_ctx;
    ethernet_xmit_t xmit;
};

static int ethernet_layer_init(void);
static const EthernetHandler *find_handler(uint16_t ether_type);
static const EthernetDevice *find_device(const char *name);
static const EthernetDevice *find_device_by_phy(const char *name);

DECLARE_DRIVER(ethernet_layer, ethernet_layer_init, STAGE_SECOND);

static basic_queue<NetworkBuffer *> gRxQueue(RX_QUEUE_SIZE);
static basic_queue<NetworkBuffer *> gTxQueue(TX_QUEUE_SIZE);
static std::vector<EthernetHandler> gHandlers;
static std::vector<EthernetDevice> gDevices;
static std::atomic<int> gDevIndex(0);

static int generate_dev_index(void) {
    return gDevIndex.fetch_add(1, std::memory_order::memory_order_relaxed);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

_GLIBCXX_NORETURN
static int ethernet_rx_main(void* argument) {
    while (1) {
        NetworkBuffer *nbuf = NULL;
        bool result = gRxQueue.pop(nbuf, true);
        if (!result) {
            continue;
        }

        const ethernet_t *header = ethernet_hdr(nbuf);
        const EthernetHandler *handler = find_handler(header->type);
        if (nullptr == handler) {
            kprintf("EthernetRx: could not find handler for ethernet type %04x. dropping packet.\n", header->type);
            nbuf_free(nbuf);
            continue;
        }

        const EthernetDevice *ethernetDevice = find_device_by_phy(nbuf_device(nbuf));
        assert(ethernetDevice != nullptr);

        // check if this frame is a unicast
        if (0 == (header->dst[0] & 1)) {
            // if it is: make sure that it fits the receiving phy device mac address.
            if (0 != memcmp(header->dst, ethernetDevice->mac, ETH_ALEN)) {
                nbuf_free(nbuf);
                continue;
            }
        }

        // the handler wants to know the eth%d device name, not the underlying phy device..
        // as he cant do anything with it..
        nbuf_set_device(nbuf, ethernetDevice->name);
        // set the l3 header offset
        nbuf_set_l3(nbuf, (void *) header->data, header->type);

        handler->handler(handler->user_ctx, nbuf);
    }
}

_GLIBCXX_NORETURN
static int ethernet_tx_main(void* argument) {
    while (true) {
        NetworkBuffer *nbuf = NULL;
        if (!gTxQueue.pop(nbuf, true)) {
            continue;
        }

        const EthernetDevice *ethernetDevice = find_device(nbuf_device(nbuf));
        if (nullptr == ethernetDevice) {
            nbuf_free(nbuf);
            continue;
        }

        ethernet_t *header = ethernet_hdr(nbuf);
        memcpy(header->src, ethernetDevice->mac, 6);
        // header->type was already set in ethernet_alloc_nbuf()
        assert(header->type == nbuf->l3_proto);

        ethernetDevice->xmit(ethernetDevice->user_ctx, nbuf);
        nbuf_free(nbuf);
    }
}

#pragma clang diagnostic pop

void ethernet_absorb_packet(NetworkBuffer *nbuf, const char *phyName) {
    nbuf_set_device(nbuf, phyName);
    nbuf_set_l2(nbuf, nbuf_data(nbuf));

    if (!gRxQueue.push(nbuf, false)) {
        // reverse the queue's ref count
        nbuf_free(nbuf);
    }
}

int ethernet_register_handler(uint16_t ether_type, ethernet_handler_t handler, void *user_ctx) {
    if (nullptr != find_handler(ether_type)) {
        return 0;
    }

    gHandlers.push_back({ether_type, user_ctx, handler});
    return 1;
}

int ethernet_register_device(const char *phyName, uint8_t *mac, void *user_ctx, ethernet_xmit_t xmit) {
    if (nullptr != find_device_by_phy(phyName)) {
        return 0;
    }

    EthernetDevice dev;
    dev.phy = phyName;
    dev.user_ctx = user_ctx;
    dev.xmit = xmit;
    memcpy(dev.mac, mac, 6);
    sprintf(dev.name, "eth%d", generate_dev_index());

    kprintf("eth: registered ethernet device %s as %s\n", phyName, dev.name);

    gDevices.push_back(dev);
    return 1;
}

int ethernet_send_packet(NetworkBuffer *nbuf, MacAddress dst) {
    ethernet_t *header = ethernet_hdr(nbuf);
    memcpy(header->dst, dst, 6);

    if (!gTxQueue.push(nbuf, false)) {
        nbuf_free(nbuf);
        return 0;
    }

    return 1;
}

int ethernet_device_hwaddr(const char *devName, MacAddress outMac) {
    if (!devName || !outMac) {
        return 0;
    }

    const EthernetDevice *ethernetDevice = find_device(devName);
    if (nullptr == ethernetDevice) {
        return 0;
    }

    memcpy(outMac, ethernetDevice->mac, 6);
    return 1;
}

NetworkBuffer *ethernet_alloc_nbuf(const char *device, uint16_t proto, size_t size) {
    NetworkBuffer *nbuf = nbuf_alloc(sizeof(ethernet_t) + size);
    if (!nbuf) {
        return nbuf;
    }

    nbuf_set_device(nbuf, device);
    nbuf_set_l2(nbuf, nbuf_data(nbuf));
    nbuf_set_l3(nbuf, (char *) nbuf_data(nbuf) + sizeof(ethernet_t), proto);
    nbuf_set_size(nbuf, nbuf_capacity(nbuf));

    ethernet_hdr(nbuf)->type = proto;
    return nbuf;
}

static const EthernetHandler *find_handler(uint16_t ether_type) {
    for (const EthernetHandler &handler : gHandlers) {
        if (handler.ether_type == ether_type) {
            return &handler;
        }
    }

    return nullptr;
}

static const EthernetDevice *find_device(const char *name) {
    for (const EthernetDevice &device : gDevices) {
        if (0 == strcmp(name, device.name)) {
            return &device;
        }
    }

    return nullptr;
}

static const EthernetDevice *find_device_by_phy(const char *name) {
    for (const EthernetDevice &device : gDevices) {
        if (0 == strcmp(name, device.phy)) {
            return &device;
        }
    }

    return nullptr;
}

static int ethernet_layer_init(void) {
    ProcessProvider::instance().createKernelThread("EthernetRx", ethernet_rx_main, nullptr, 8192);
    ProcessProvider::instance().createKernelThread("EthernetTx", ethernet_tx_main, nullptr, 8192);
    return 0;
}
