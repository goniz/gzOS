#include <platform/drivers.h>
#include <lib/primitives/basic_queue.h>
#include <lib/syscall/syscall.h>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <atomic>
#include "ethernet_layer.h"
#include "packet_pool.h"

struct EthernetHandler {
    uint16_t ether_type;
    void* user_ctx;
    ethernet_handler_t handler;
};

struct EthernetDevice {
    char name[16];
    const char* phy;
    uint8_t mac[6];
    void* user_ctx;
    ethernet_xmit_t xmit;
};

_GLIBCXX_NORETURN
static int ethernet_rx_main(int argc, const char **argv);
static int ethernet_layer_init(void);
static const EthernetHandler* find_handler(uint16_t ether_type);
static const EthernetDevice* find_device(const char* name);
static const EthernetDevice* find_device_by_phy(const char* name);

static basic_queue<IncomingPacketBuffer> gRxQueue(RX_QUEUE_SIZE);
static std::vector<EthernetHandler> gHandlers;
static std::vector<EthernetDevice> gDevices;
static std::atomic<int> gDevIndex(0);

static int generate_dev_index(void) {
    return gDevIndex.fetch_add(1, std::memory_order::memory_order_relaxed);
}

DECLARE_DRIVER(ethernet_layer, ethernet_layer_init, STAGE_SECOND);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
_GLIBCXX_NORETURN
static int ethernet_rx_main(int argc, const char **argv)
{
    while (1) {
        IncomingPacketBuffer inpkt;
        if (gRxQueue.pop(inpkt, true)) {
            kprintf("ethernet_rx_main\n");
            const EthernetHandler* handler = find_handler(inpkt.header->type);
            if (nullptr == handler) {
                kprintf("EthernetRx: could not find handler for ethernet type %04x. dropping packet.\n", inpkt.header->type);
                packet_pool_free_underlying_buffer(&inpkt.buffer);
            } else {
                inpkt.buffer = packet_pool_view_of_buffer(inpkt.buffer.underlyingBuffer, sizeof(ethernet_t), 0);
                handler->handler(handler->user_ctx, &inpkt);
            }
        }
    }
}
#pragma clang diagnostic pop

void ethernet_absorbe_packet(const char* phy, PacketBuffer packetBuffer) {
    IncomingPacketBuffer incomingPacketBuffer;
    incomingPacketBuffer.buffer = packet_pool_view_of_buffer(packetBuffer, 0, 0);
    incomingPacketBuffer.header = (ethernet_t*)packetBuffer.buffer;
    incomingPacketBuffer.phy = phy;

    kprintf("ethernet_absorbe_packet\n");

    if (!gRxQueue.push(incomingPacketBuffer, false)) {
        // free the packet if we cant push it..
        packet_pool_free(&packetBuffer);
    }
}

int ethernet_register_handler(uint16_t ether_type, ethernet_handler_t handler, void *user_ctx) {
    if (nullptr != find_handler(ether_type)) {
        return 0;
    }

    gHandlers.push_back({ether_type, user_ctx, handler});
    return 1;
}

int ethernet_register_device(const char* phyName, uint8_t* mac, void* user_ctx, ethernet_xmit_t xmit) {
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

int ethernet_send_packet(const char* devName, uint8_t* dst, uint16_t ether_type, PacketView* packetView) {
    const EthernetDevice* ethernetDevice = find_device(devName);
    if (nullptr == ethernetDevice) {
        return 0;
    }

    PacketBuffer ethPacket = packet_pool_alloc(sizeof(ethernet_t) + packetView->size);
    if (!ethPacket.buffer) {
        packet_pool_free_underlying_buffer(packetView);
        return 0;
    }

    ethernet_t* header = (ethernet_t *) ethPacket.buffer;
    memcpy(header->dst, dst, 6);
    memcpy(header->src, ethernetDevice->mac, 6);
    header->type = ether_type;
    memcpy(header->data, packetView->buffer, packetView->size);
    packet_pool_free_underlying_buffer(packetView);

    ethernetDevice->xmit(ethernetDevice->user_ctx, &ethPacket);
    packet_pool_free(&ethPacket);
    return 1;
}

static const EthernetHandler* find_handler(uint16_t ether_type) {
    for (const EthernetHandler& handler : gHandlers) {
        if (handler.ether_type == ether_type) {
            return &handler;
        }
    }

    return nullptr;
}

static const EthernetDevice* find_device(const char* name) {
    for (const EthernetDevice& device : gDevices) {
        if (0 == strcmp(name, device.name)) {
            return &device;
        }
    }

    return nullptr;
}

static const EthernetDevice* find_device_by_phy(const char* name) {
    for (const EthernetDevice& device : gDevices) {
        if (0 == strcmp(name, device.phy)) {
            return &device;
        }
    }

    return nullptr;
}

static int ethernet_layer_init(void)
{
    std::vector<const char*> args{};
    syscall(SYS_NR_CREATE_RESPONSIVE_PROC, "EthernetRx", ethernet_rx_main, args.size(), args.data(), 8096);
    return 0;
}