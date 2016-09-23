#include <platform/kprintf.h>
#include <algorithm>
#include "icmp.h"
#include "nbuf.h"
#include "ethernet.h"
#include "ip.h"
#include "checksum.h"

static void icmpv4_reply(NetworkBuffer *packet);

int icmp_input(NetworkBuffer* packet)
{
    int ret = 0;
    icmp_v4_t* icmp = icmp_v4_hdr(packet);

    if (nbuf_size_from(packet, icmp) < sizeof(icmp_v4_t)) {
        kprintf("icmp: packet is too small, dropping. (buf %d ip %d)\n", nbuf_size_from(packet, icmp), sizeof(icmp_v4_t));
        goto error;
    }

    switch (icmp->type)
    {
        case ICMP_V4_ECHO:
            icmpv4_reply(packet);
        default:
            break;
    }

    goto exit;

error:
    ret = -1;

exit:
    nbuf_free(packet);
    return ret;
}

static void icmpv4_reply(NetworkBuffer* packet)
{
    ethernet_t* eth = ethernet_hdr(packet);
    iphdr_t* ip = ip_hdr(packet);
    icmp_v4_pingpong_t* icmp = (icmp_v4_pingpong_t *) icmp_v4_hdr(packet);

    std::swap(eth->dst, eth->src);
    IpAddress dst = ip->saddr;
    IpAddress src = ip->daddr;
    ip->daddr = dst;
    ip->saddr = src;

    icmp->header.csum = 0;
    icmp->header.type = ICMP_V4_REPLY;
    icmp->header.csum = checksum(icmp, (int) nbuf_size_from(packet, icmp));

    ip_output(packet);
}