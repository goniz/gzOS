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
#include "ip.h"

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

//! \brief
//!     Calculate the UDP checksum (calculated with the whole
//!     packet).
//! \param buff The UDP packet.
//! \param len The UDP packet length.
//! \param src_addr The IP source address (in network format).
//! \param dest_addr The IP destination address (in network format).
//! \return The result of the checksum.
static uint16_t udp_checksum(const void *buff, size_t len, uint32_t src_addr, uint32_t dest_addr)
{
        const uint16_t *buf = (const uint16_t *) buff;
        uint16_t *ip_src = (uint16_t *)&src_addr;
        uint16_t *ip_dst = (uint16_t *)&dest_addr;
        uint32_t sum = 0;
        size_t length = len;

        // Calculate the sum                                            //
        while (len > 1)
        {
                sum += *buf++;
                if (sum & 0x80000000)
                        sum = (sum & 0xFFFF) + (sum >> 16);
                len -= 2;
        }

        if ( len & 1 )
                // Add the padding if the packet length is odd          //
                sum += *((uint8_t *)buf);

        // Add the pseudo-header                                        //
        sum += *(ip_src++);
        sum += *ip_src;

        sum += *(ip_dst++);
        sum += *ip_dst;

        sum += htons(IPPROTO_UDP);
        sum += htons(length);

        // Add the carries                                              //
        while (sum >> 16)
                sum = (sum & 0xFFFF) + (sum >> 16);

        // Return the one's complement of sum                           //
        return ( (uint16_t)(~sum)  );
}

static uint16_t udp_checksum(const NetworkBuffer* nbuf)
{
    iphdr_t* iphdr = ip_hdr(nbuf);
    udp_t* udp = udp_hdr(nbuf);

    uint16_t oldcsum = udp->csum;
    udp->csum = 0;

    const auto csum = udp_checksum(udp, ntohs(udp->length), iphdr->saddr, iphdr->daddr);

    udp->csum = oldcsum;

    return csum;
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
        return false;
    }

    return true;
}

DECLARE_DRIVER(udp_proto, udp_proto_init, STAGE_SECOND + 1);
