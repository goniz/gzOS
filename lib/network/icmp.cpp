#include <platform/kprintf.h>
#include <algorithm>
#include <platform/cpu.h>
#include "icmp.h"
#include "checksum.h"
#include "ip.h"
#include "nbuf.h"

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
            break;

        default:
            nbuf_free(packet);
            break;
    }

    goto exit;

error:
    ret = -1;
    nbuf_free(packet);

exit:
    return ret;
}

int icmp_reject(uint8_t icmp_type, uint8_t icmp_code, const NetworkBuffer *echoPacket)
{
    iphdr_t* echoIpHeader = ip_hdr(echoPacket);
    size_t echoPacketSize = nbuf_size_from(echoPacket, echoIpHeader);
    IpAddress dstIp = ntohl(echoIpHeader->saddr);
    uint16_t packetSize = sizeof(icmp_v4_t) + 4 + echoPacketSize;
    NetworkBuffer* outputNbuf = ip_alloc_nbuf(dstIp, 64, IPPROTO_ICMP, packetSize);

    if (!outputNbuf) {
        return -1;
    }

    icmp_v4_t* icmp = icmp_v4_hdr(outputNbuf);
    icmp->type = icmp_type;
    icmp->code = icmp_code;
    icmp->csum = 0;
    memset(icmp->data, 0, 4);
    memcpy(icmp->data + 4, echoIpHeader, echoPacketSize);
    icmp->csum = checksum(icmp, packetSize);

    return ip_output(outputNbuf);
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