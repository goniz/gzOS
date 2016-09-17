#include <platform/drivers.h>
#include <cstdlib>
#include <platform/kprintf.h>
#include <platform/cpu.h>
#include "ethernet.h"
#include "ip.h"
#include "nbuf.h"
#include "checksum.h"

static int ip_layer_init(void);
static void ip_handler(void* user_ctx, NetworkBuffer* incomingPacket);
static bool ip_is_valid(iphdr_t* iphdr);

DECLARE_DRIVER(ip_layer, ip_layer_init, STAGE_SECOND + 1);

int ip_recv(NetworkBuffer* incomingPacket)
{
    int ret = 0;
    iphdr_t* iphdr = ip_hdr(incomingPacket);

    if (nbuf_size_from(incomingPacket, iphdr) < sizeof(iphdr_t)) {
        kprintf("ip: packet is too small, dropping. (buf %d ip %d)\n", nbuf_size_from(incomingPacket, iphdr), sizeof(iphdr_t));
        goto error;
    }

    if (4 != iphdr->version) {
        goto error;
    }

    iphdr->len = ntohs(iphdr->len);
    iphdr->id = ntohs(iphdr->id);
    iphdr->frag_offset = ntohs(iphdr->frag_offset);
    iphdr->csum = ntohs(iphdr->csum);
    iphdr->saddr = ntohl(iphdr->saddr);
    iphdr->daddr = ntohl(iphdr->daddr);

    if (!ip_is_valid(iphdr)) {
        goto error;
    }

    kprintf("ip: received an IP datagram %08x --> %08x (%d)\n", iphdr->saddr, iphdr->daddr, iphdr->proto);
    switch (iphdr->proto)
    {
        case IPPROTO_ICMP:
            break;

        default:
            break;
    }

    goto exit;

error:
    ret = -1;

exit:
    nbuf_free(incomingPacket);
    return ret;
}

int ip_output()
{
    return -1;
}

static int ip_layer_init(void)
{
    ethernet_register_handler(ETH_P_IPv4, ip_handler, NULL);
    return 0;
}

static void ip_handler(void* user_ctx, NetworkBuffer* incomingPacket)
{
    (void)user_ctx;

    ip_recv(incomingPacket);
}

static bool ip_is_valid(iphdr_t* iphdr) {
    if (5 > iphdr->ihl) {
        kprintf("ip: IPv4 header length must be at least 5\n");
        return false;
    }

    if (IP_FLAGS(iphdr) & IP_FLAGS_MORE_FRAGMENTS) {
        kprintf("ip: Fragments are not supported yet..\n");
        return false;
    }

    uint16_t orig_csum = iphdr->csum;
    iphdr->csum = 0;

    uint16_t new_csum = checksum(iphdr, iphdr->ihl * 4);
    iphdr->csum = orig_csum;

    if (orig_csum != new_csum) {
        kprintf("ip: checksum mismatch (got %04x expected %04x)\n", orig_csum, new_csum);
        return false;
    }

    return true;
}