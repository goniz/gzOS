#include <lib/primitives/hashmap.h>
#include <platform/kprintf.h>
#include <platform/cpu.h>
#include <lib/primitives/basic_queue.h>
#include <platform/drivers.h>
#include <lib/primitives/hexdump.h>
#include "lib/network/udp/udp.h"
#include "lib/network/icmp/icmp.h"
#include "lib/network/socket.h"
#include "lib/network/udp/udp_socket.h"
#include "lib/network/checksum.h"

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

    const auto expected = udp_checksum(nbuf);
    const auto found = ntohs(udp->csum);
    if (found != expected) {
        kprintf("udp: invalid checksum - expected %04x found %04x\n", expected, found);

        const iphdr_t* ip = ip_hdr(nbuf);
        hexDump(NULL, (void*)ip, ip->len);
        return false;
    }

    return true;
}

DECLARE_DRIVER(udp_proto, udp_proto_init, STAGE_SECOND + 1);
