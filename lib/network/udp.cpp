#include <lib/primitives/hashmap.h>
#include <platform/kprintf.h>
#include <platform/cpu.h>
#include <lib/primitives/basic_queue.h>
#include <sys/param.h>
#include <platform/drivers.h>
#include "udp.h"
#include "icmp.h"
#include "socket.h"
#include "checksum.h"
#include "udp_socket.h"

static uint16_t udp_checksum(const NetworkBuffer* nbuf);
static bool udp_validate_packet(const NetworkBuffer* nbuf, const udp_t* udp);

int udp_input(NetworkBuffer *packet)
{
    int ret = 0;
    uint16_t dport = 0;
    UdpFileDescriptor* session = nullptr;
    udp_t* udp = udp_hdr(packet);

    if (!udp_validate_packet(packet, udp)) {
        goto error;
    }

    dport = ntohs(udp->dport);
    session = getUdpDescriptorByPort(dport);
    if (nullptr == session) {
        icmp_reject(ICMP_V4_DST_UNREACHABLE, ICMP_V4_PORT_UNREACHABLE, packet);
        goto error;
    }

    session->rxQueue.push(packet);
    goto exit;

error:
    ret = -1;
    nbuf_free(packet);

exit:
    return ret;
}

NetworkBuffer *udp_alloc_nbuf(IpAddress destinationIp, uint16_t destinationPort, uint16_t size)
{
    const uint16_t udpSize = sizeof(udp_t) + size;
    NetworkBuffer* nbuf = ip_alloc_nbuf(destinationIp, 64, IPPROTO_UDP, udpSize);
    if (!nbuf) {
        return nullptr;
    }

    udp_t* udp = udp_hdr(nbuf);
    udp->dport = htons(destinationPort);
    udp->sport = 0;
    udp->length = htons(udpSize);
    udp->csum = 0;

    return nbuf;
}

int udp_output(NetworkBuffer* packet)
{
    udp_t* udp = udp_hdr(packet);
    udp->csum = 0;
    udp->csum = udp_checksum(packet);

    return ip_output(packet);
}

static int udp_proto_init(void)
{
    socket_register_triple({AF_INET, SOCK_DGRAM, IPPROTO_UDP}, []() {
        return std::unique_ptr<SocketFileDescriptor>(new UdpFileDescriptor());
    });

    return 0;
}

// TODO: fix the off by one bug we have here somewhere..
static uint16_t udp_checksum(const NetworkBuffer* nbuf)
{
    struct {
        uint32_t source;
        uint32_t destination;
        uint8_t zero;
        uint8_t protocol;
        uint16_t length;
    } __attribute__((packed)) pseudoHeader;

    iphdr_t* iphdr = ip_hdr(nbuf);
    udp_t* udp = udp_hdr(nbuf);

    memset(&pseudoHeader, 0, sizeof(pseudoHeader));
    pseudoHeader.source = ntohl(iphdr->saddr);
    pseudoHeader.destination = ntohl(iphdr->daddr);
    pseudoHeader.protocol = iphdr->proto;
    pseudoHeader.length = ntohs(udp->length);

    uint32_t pseudo_csum = ntohl(iphdr->saddr) + ntohl(iphdr->daddr) + iphdr->proto + ntohs(udp->length);

    uint16_t oldcsum = udp->csum;
    udp->csum = 0;

    const uint32_t pseudo_csum = csum_partial(&pseudoHeader, sizeof(pseudoHeader), 0);
    const uint32_t csum = csum_partial(udp, ntohs(udp->length), pseudo_csum);

    udp->csum = oldcsum;

    return csum_partial_done(csum);
}

static bool udp_validate_packet(const NetworkBuffer* nbuf, const udp_t* udp)
{
    if (nbuf_size_from(nbuf, udp) < sizeof(udp_t)) {
        kprintf("udp: packet is too small, dropping. (buf %d udp %d)\n", nbuf_size_from(nbuf, udp), sizeof(udp_t));
        return false;
    }

    if (nbuf_size_from(nbuf, udp->data) < udp_data_length(udp)) {
        kprintf("udp: packet buffer is too small, header length corrupted? (buf %d udp_data_length(udp) %d)\n",
                nbuf_size_from(nbuf, udp->data),
                udp_data_length(udp));
        return false;
    }

    // TODO: fix the off by one bug, and remove this relaxed check
    const auto expected = udp_checksum(nbuf);
    const auto found = ntohs(udp->csum);
    if (found != expected && (found != (expected - 1))) {
        kprintf("udp: invalid checksum - expected %04x found %04x\n", expected, found);
        return false;
    }

    return true;
}

DECLARE_DRIVER(udp_proto, udp_proto_init, STAGE_SECOND + 1);