#ifndef GZOS_TCP_H
#define GZOS_TCP_H

#include <lib/network/nbuf.h>
#include <lib/network/ip/ip.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t ns: 1,
            cwr : 1,
            ece : 1,
            urg : 1,
            ack : 1,
            psh : 1,
            rst : 1,
            syn : 1,
            fin : 1;
} __attribute__((packed)) tcp_flags_t;

#define TCP_FLAGS_FIN   (1 << 0)
#define TCP_FLAGS_SYN   (1 << 1)
#define TCP_FLAGS_RESET (1 << 2)
#define TCP_FLAGS_PUSH  (1 << 3)
#define TCP_FLAGS_ACK   (1 << 4)
#define TCP_FLAGS_URG   (1 << 5)
#define TCP_FLAGS_ECE   (1 << 6)
#define TCP_FLAGS_CWR   (1 << 7)
#define TCP_FALGS_NS    (1 << 8)

typedef struct {
    uint16_t sport;
    uint16_t dport;
    uint32_t seq;
    uint32_t ack_seq;
    uint8_t hl : 4;
    uint8_t rsvd : 3;
    uint32_t flags : 9;
    uint16_t win;
    uint16_t csum;
    uint16_t urp;
    uint8_t data[0];
} __attribute__((packed)) tcp_t;

int tcp_input(NetworkBuffer* packet);

static inline tcp_t* tcp_hdr(const NetworkBuffer* nbuf) {
    assert(nbuf->l4_proto == IPPROTO_TCP);
    assert(NULL != nbuf->l4_offset);
    return (tcp_t*)(nbuf->l4_offset);
}

static inline uint16_t tcp_header_length(const tcp_t* hdr) {
    return (uint16_t)(hdr->hl * sizeof(uint32_t));
}

static inline uint16_t tcp_data_length(const iphdr_t* ip, const tcp_t* tcp) {
    return ip_data_length(ip) - tcp_header_length(tcp);
}

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_TCP_H
