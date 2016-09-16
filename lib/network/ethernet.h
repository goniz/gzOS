#ifndef GZOS_ETHERNET_LAYER_H
#define GZOS_ETHERNET_LAYER_H

#include "packet_pool.h"

#define RX_QUEUE_SIZE       (32)

#ifdef __cplusplus
extern "C" {
#endif

#define ETH_ALEN (6)
#define MAC_FORMAT      "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARGUMENT(x) ((uint8_t*)(x))[0], \
                        ((uint8_t*)(x))[1], \
                        ((uint8_t*)(x))[2], \
                        ((uint8_t*)(x))[3], \
                        ((uint8_t*)(x))[4], \
                        ((uint8_t*)(x))[5]


typedef uint8_t MacAddress[ETH_ALEN];

typedef struct {
    MacAddress dst;
    MacAddress src;
    uint16_t type;
    uint8_t data[0];
} ethernet_t;

typedef struct {
    PacketView buffer;
    const char* inputDevice;
    ethernet_t* header;
} IncomingPacketBuffer;

typedef enum {
    ETH_P_ARP = 0x0806,
    ETH_P_IPv4 = 0x0800
} ethernet_type_t;

typedef void (*ethernet_handler_t)(void* user_ctx, IncomingPacketBuffer* incomingPacket);
typedef int (*ethernet_xmit_t)(void* user_ctx, PacketBuffer* packetBuffer);

void ethernet_absorb_packet(const char *phy, PacketBuffer packetBuffer);
int ethernet_send_packet(const char* devName, MacAddress dst, uint16_t ether_type, PacketView* packetView);

int ethernet_register_handler(uint16_t ether_type, ethernet_handler_t handler, void* user_ctx);
int ethernet_register_device(const char* phyName, uint8_t* mac, void* user_ctx, ethernet_xmit_t xmit);
int ethernet_device_hwaddr(const char* devName, void* buf, size_t* size);

#ifdef __cplusplus
};
#endif
#endif //GZOS_ETHERNET_LAYER_H
