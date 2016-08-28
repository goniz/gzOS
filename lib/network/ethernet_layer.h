#ifndef GZOS_ETHERNET_LAYER_H
#define GZOS_ETHERNET_LAYER_H

#include "packet_pool.h"

#define RX_QUEUE_SIZE       (32)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;
    uint8_t data[0];
} ethernet_t;

typedef struct {
    PacketView buffer;
    const char* phy;
    ethernet_t* header;
} IncomingPacketBuffer;

typedef void (*ethernet_handler_t)(void* user_ctx, IncomingPacketBuffer* incomingPacket);
typedef int (*ethernet_xmit_t)(void* user_ctx, PacketBuffer* packetBuffer);

void ethernet_absorbe_packet(const char* phy, PacketBuffer packetBuffer);
int ethernet_register_handler(uint16_t ether_type, ethernet_handler_t handler, void* user_ctx);
int ethernet_register_device(const char* phyName, uint8_t* mac, void* user_ctx, ethernet_xmit_t xmit);
int ethernet_send_packet(const char* devName, uint8_t* dst, uint16_t ether_type, PacketView* packetView);

#ifdef __cplusplus
};
#endif
#endif //GZOS_ETHERNET_LAYER_H
