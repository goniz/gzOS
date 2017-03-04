#include <assert.h>
#include <machine/endian.h>
#include <platform/cpu.h>
#include "lib/network/checksum.h"
#include "lib/network/ip/ip.h"
#include "lib/network/udp/udp.h"
#include "lib/network/tcp/tcp.h"

static uint32_t sum_every_16bits(const void* addr, int count)
{
    register uint32_t sum = 0;
    const uint16_t * ptr = addr;

    while( count > 1 )  {
        /*  This is the inner loop */
        sum += * ptr++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if( count > 0 ) {
#if BIG_ENDIAN == BYTE_ORDER
        sum += ((*(uint8_t*) ptr) << 8);
#else
        sum += *(uint8_t*) ptr;
#endif
    }

    return sum;
}

uint16_t checksum(const void* addr, int count, int start_sum)
{
    /* Compute Internet Checksum for "count" bytes
     *         beginning at location "addr".
     * Taken from https://tools.ietf.org/html/rfc1071
     */
    uint32_t sum = start_sum;

    sum += sum_every_16bits(addr, count);

    /*  Fold 32-bit sum to 16 bits */
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

static int tcp_udp_checksum(uint32_t saddr, uint32_t daddr, uint8_t proto,
                     uint8_t *data, uint16_t len)
{
    uint32_t sum = 0;

    sum += saddr;
    sum += daddr;
    sum += htons(proto);
    sum += htons(len);

    return checksum(data, len, sum);
}

uint16_t udp_checksum(const NetworkBuffer* nbuf) {
    iphdr_t* iphdr = ip_hdr(nbuf);
    udp_t* udp = udp_hdr(nbuf);

    uint16_t oldcsum = udp->csum;
    udp->csum = 0;

    uint16_t csum = tcp_udp_checksum(iphdr->saddr, iphdr->daddr, IPPROTO_UDP, (uint8_t*)udp, ntohs(udp->length));

    udp->csum = oldcsum;
    return csum;
}

uint16_t tcp_checksum(const NetworkBuffer* nbuf) {
    iphdr_t* iphdr = ip_hdr(nbuf);
    tcp_t* tcp = tcp_hdr(nbuf);

    uint16_t oldcsum = tcp->csum;
    tcp->csum = 0;

    uint16_t csum = tcp_udp_checksum(iphdr->saddr, iphdr->daddr, IPPROTO_TCP, (uint8_t*)tcp, ip_data_length(iphdr));

    tcp->csum = oldcsum;
    return csum;
}