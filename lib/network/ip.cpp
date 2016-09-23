#include <platform/drivers.h>
#include <cstdlib>
#include <platform/kprintf.h>
#include <platform/cpu.h>
#include <atomic>
#include <vector>
#include "ethernet.h"
#include "ip.h"
#include "nbuf.h"
#include "checksum.h"
#include "route.h"
#include "interface.h"
#include "arp.h"
#include "icmp.h"

static int ip_layer_init(void);
static void ip_handler(void* user_ctx, NetworkBuffer* incomingPacket);
static bool ip_is_valid(iphdr_t* iphdr);
static void ip_hint_arp_cache(NetworkBuffer *packet);

static std::atomic<uint16_t> gId(0);

static uint16_t nextId(void) {
    return gId.fetch_add(1, std::memory_order::memory_order_relaxed);
}

DECLARE_DRIVER(ip_layer, ip_layer_init, STAGE_SECOND + 1);

int ip_input(NetworkBuffer *incomingPacket)
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

    nbuf_set_l4(incomingPacket, iphdr->data, iphdr->proto);
    ip_hint_arp_cache(incomingPacket);

//    kprintf("ip: received an IP datagram %08x --> %08x (%d)\n", iphdr->saddr, iphdr->daddr, iphdr->proto);
//    kprintf("ip: pkt refcnt %d\n", incomingPacket->refcnt);
    switch (iphdr->proto)
    {
        case IPPROTO_ICMP:
            icmp_input(incomingPacket);
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

static void ip_hint_arp_cache(NetworkBuffer *packet) {
    const char* device = nbuf_device(packet);
    ethernet_t* eth = ethernet_hdr(packet);
    iphdr_t* ip = ip_hdr(packet);

    arp_set_entry(device, eth->src, ip->saddr);
}

NetworkBuffer* ip_alloc_nbuf(IpAddress dst, uint8_t ttl, uint16_t proto, uint16_t size) {
    route_t route;
    if (0 != ip_route_lookup(dst, &route)) {
        return NULL;
    }

    NetworkBuffer *nbuf = ethernet_alloc_nbuf(route.iface->device, ETH_P_IPv4, sizeof(iphdr_t) + size);
    if (!nbuf) {
        return nbuf;
    }

    iphdr_t* iphdr = ip_hdr(nbuf);
    memset(iphdr, 0, sizeof(*iphdr));
    iphdr->version = 4;
    iphdr->ihl = 5;
    iphdr->id = htons(nextId());
    iphdr->ttl = ttl;
    iphdr->len = htons(sizeof(iphdr_t) + size);
    iphdr->saddr = htonl(route.iface->ipv4.address);
    iphdr->daddr = htonl(dst);
    nbuf_set_l4(nbuf, iphdr->data, proto);

    return nbuf;
}

struct ArpResolveContext {
    NetworkBuffer* ipPacket;
};

static void arp_resolve_cb(NetworkBuffer* arpContext, int result)
{
    MacAddress mac{};
    ArpResolveContext* ctx = (ArpResolveContext *) nbuf_data(arpContext);
    iphdr_t* iphdr = ip_hdr(ctx->ipPacket);

    if (0 == result) {
        goto error;
    }

    if (0 != arp_get_hwaddr(ntohl(iphdr->daddr), mac)) {
        goto error;
    }

    ethernet_send_packet(ctx->ipPacket, mac);
    goto exit;

error:
    nbuf_free(ctx->ipPacket);

exit:
    nbuf_free(arpContext);
}

// no ip options for now..
int ip_output(NetworkBuffer* packet)
{
    iphdr_t* iphdr = ip_hdr(packet);
    iphdr->csum = 0;
    iphdr->csum = checksum(iphdr, iphdr->ihl * 4);

    nbuf_use(packet);

    NetworkBuffer* arpContextNbuf = nbuf_alloc(sizeof(ArpResolveContext));
    if (!arpContextNbuf) {
        nbuf_free(packet);
        return -1;
    }

    ArpResolveContext* arpContext = (ArpResolveContext *) nbuf_data(arpContextNbuf);
    arpContext->ipPacket = packet;

    if (0 != arp_resolve(ntohl(iphdr->daddr), (arp_resolve_cb_t)arp_resolve_cb, arpContextNbuf)) {
        nbuf_free(packet);
        nbuf_free(arpContextNbuf);
        return -1;
    }

    return 0;
}

static int ip_layer_init(void)
{
    ethernet_register_handler(ETH_P_IPv4, ip_handler, NULL);
    return 0;
}

static void ip_handler(void* user_ctx, NetworkBuffer* incomingPacket)
{
    (void)user_ctx;

    ip_input(incomingPacket);
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