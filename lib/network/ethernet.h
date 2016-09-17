#ifndef GZOS_ETHERNET_LAYER_H
#define GZOS_ETHERNET_LAYER_H

#include "nbuf.h"

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

typedef enum {
    ETH_P_ARP = 0x0806,
    ETH_P_IPv4 = 0x0800
} ethernet_type_t;

typedef void (*ethernet_handler_t)(void* user_ctx, NetworkBuffer* incomingPacket);
typedef int (*ethernet_xmit_t)(void* user_ctx, NetworkBuffer* packetBuffer);

void ethernet_absorb_packet(NetworkBuffer* nbuf, const char* phyName);
int ethernet_send_packet(NetworkBuffer* packet, MacAddress dst);

int ethernet_register_handler(uint16_t ether_type, ethernet_handler_t handler, void* user_ctx);
int ethernet_register_device(const char* phyName, uint8_t* mac, void* user_ctx, ethernet_xmit_t xmit);
int ethernet_device_hwaddr(const char* devName, void* buf, size_t* size);

NetworkBuffer* ethernet_alloc_nbuf(const char* device, uint16_t proto, size_t size);

static inline ethernet_t* ethernet_hdr(NetworkBuffer* nbuf) {
    assert(NULL != nbuf->l2_offset);
    return (ethernet_t*)(nbuf->l2_offset);
}

#ifdef __cplusplus
};
#endif
#endif //GZOS_ETHERNET_LAYER_H
