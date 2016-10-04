#ifndef GZOS_UDP_H
#define GZOS_UDP_H

#include "nbuf.h"
#include "ip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t sport;
    uint16_t dport;
    uint16_t length;
    uint16_t csum;
    uint8_t data[0];
} __attribute__((packed)) udp_t;

int udp_input(NetworkBuffer *packet);
int udp_output(NetworkBuffer* packet);
NetworkBuffer *udp_alloc_nbuf(IpAddress destinationIp, uint16_t destinationPort, uint16_t size);

static inline udp_t* udp_hdr(const NetworkBuffer* nbuf) {
    assert(nbuf->l4_proto == IPPROTO_UDP);
    assert(NULL != nbuf->l4_offset);
    return (udp_t*)(nbuf->l4_offset);
}

static inline uint16_t udp_data_length(const udp_t* hdr) {
    return hdr->length - sizeof(udp_t);
}

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_UDP_H
