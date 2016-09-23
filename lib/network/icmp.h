#ifndef GZOS_ICMP_H
#define GZOS_ICMP_H

#include <stdint.h>
#include "nbuf.h"
#include "ip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint8_t data[0];
} __attribute__((packed)) icmp_v4_t;

typedef struct {
    icmp_v4_t header;
    uint16_t id;
    uint16_t seq;
    uint64_t timestamp;
    uint8_t data[0];
} __attribute__((packed)) icmp_v4_pingpong_t;

#define ICMP_V4_REPLY           0x00
#define ICMP_V4_DST_UNREACHABLE 0x03
#define ICMP_V4_SRC_QUENCH      0x04
#define ICMP_V4_REDIRECT        0x05
#define ICMP_V4_ECHO            0x08
#define ICMP_V4_ROUTER_ADV      0x09
#define ICMP_V4_ROUTER_SOL      0x0a
#define ICMP_V4_TIMEOUT         0x0b
#define ICMP_V4_MALFORMED       0x0c

int icmp_input(NetworkBuffer *packet);

static inline icmp_v4_t* icmp_v4_hdr(NetworkBuffer* nbuf) {
    assert(nbuf->l4_proto == IPPROTO_ICMP);
    assert(NULL != nbuf->l4_offset);
    return (icmp_v4_t*)(nbuf->l4_offset);
}

#ifdef __cplusplus
}
#endif
#endif //GZOS_ICMP_H
