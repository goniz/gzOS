#ifndef GZOS_IP_H
#define GZOS_IP_H

#include <stdint.h>
#include "platform/cpu.h"
#include "lib/network/nbuf.h"
#include "lib/network/ethernet/ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t IpAddress;

// no ip options for now..
typedef struct {
#if BYTE_ORDER == BIG_ENDIAN
    uint8_t version : 4;
    uint8_t ihl : 4;
#elif BYTE_ORDER == LITTLE_ENDIAN
    uint8_t ihl : 4;
    uint8_t version : 4;
#else
    #error  "Please fix your endianess"
#endif
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t frag_offset;
    uint8_t ttl;
    uint8_t proto;
    uint16_t csum;
    uint32_t saddr;
    uint32_t daddr;
    uint8_t data[];
} __attribute__((packed)) iphdr_t;

#define IP_FLAGS(iphdr) (((iphdr)->frag_offset) >> 5)
#define IP_FLAGS_MORE_FRAGMENTS     (1 << 0)
#define IP_FLAGS_DONT_FRAGMENT      (1 << 1)

#define IPPROTO_ICMP                (1)
#define IPPROTO_TCP                 (6)
#define IPPROTO_UDP                 (17)
#define IPPROTO_RAW                 (255)

int ip_input(NetworkBuffer *incomingPacket);
int ip_output(NetworkBuffer* packet);
NetworkBuffer* ip_alloc_nbuf(IpAddress dst, uint8_t ttl, uint8_t proto, uint16_t size);

static inline iphdr_t* ip_hdr(const NetworkBuffer* nbuf) {
    assert(nbuf->l3_proto == ETH_P_IPv4);
    assert(NULL != nbuf->l3_offset);
    return (iphdr_t*)(nbuf->l3_offset);
}

static inline uint16_t ip_data_length(const iphdr_t* hdr) {
    return (uint16_t)(ntohs(hdr->len) - (hdr->ihl * sizeof(uint32_t)));
}

static inline IpAddress ip_src(const iphdr_t* ip) {
    return ntohl(ip->saddr);
}

static inline IpAddress ip_dst(const iphdr_t* ip) {
    return ntohl(ip->daddr);
}

#ifdef __cplusplus
}
#endif
#endif //GZOS_IP_H
