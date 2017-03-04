#include <cstring>
#include <platform/drivers.h>
#include <platform/cpu.h>
#include <lib/kernel/sched/scheduler.h>
#include "lib/network/nbuf.h"
#include "lib/network/ethernet/ethernet.h"
#include "lib/network/arp/arp.h"
#include "lib/network/interface.h"
#include "lib/network/route.h"

static int arp_layer_init(void);
static timeout_callback_ret arp_timeout_callback(void* scheduler, void* arg);
static void arp_handler(void* user_ctx, NetworkBuffer* nbuf);
static bool arp_is_valid(arp_t *arp);
static int arp_handle_request(arp_t *arp, const char *inputDevice);
static int arp_handle_response(arp_t *arp, const char *string);

DECLARE_DRIVER(arp_layer, arp_layer_init, STAGE_SECOND + 1);

struct ArpCacheEntry {
    MacAddress mac = {0};
    IpAddress ip = 0;
    char device[16] = {0};

    int timeout_seconds = 300;
    bool is_static = false;
};

static HashMap<IpAddress, ArpCacheEntry> _arp_cache;
static InterruptsMutex _mutex;

static int arp_layer_init(void)
{
    scheduler_set_timeout(1000, arp_timeout_callback, NULL);
    ethernet_register_handler(ETH_P_ARP, arp_handler, NULL);
    return 0;
};

static void arp_handler(void* user_ctx, NetworkBuffer* nbuf)
{
    (void)user_ctx;
    arp_t* arp = arp_hdr(nbuf);
    const char* inputDevice = nbuf->device;

    if (nbuf_size_from(nbuf, arp) < sizeof(arp_t)) {
        kprintf("arp: packet is too small, dropping. (buf %d arp %d)\n", nbuf_size_from(nbuf, arp), sizeof(arp_t));
        nbuf_free(nbuf);
        return;
    }

    arp->hardware_type = ntohs(arp->hardware_type);
    arp->operation = ntohs(arp->operation);
    arp->protocol_type = ntohs(arp->protocol_type);
    arp->sender_ip = ntohl(arp->sender_ip);
    arp->target_ip = ntohl(arp->target_ip);

    if (!arp_is_valid(arp)) {
        kprintf("arp: invalid packet structure, dropping.\n");
        nbuf_free(nbuf);
        return;
    }

    int should_free = 0;
    if (ARP_OP_REQUEST == arp->operation) {
        should_free = arp_handle_request(arp, inputDevice);
    } else {
        should_free = arp_handle_response(arp, inputDevice);
    }

    if (should_free) {
        nbuf_free(nbuf);
    }
}

static int arp_handle_response(arp_t *arp, const char* inputDevice)
{
    arp_set_entry(inputDevice, arp->sender_mac, ntohl(arp->sender_ip));
    return 1;
}

static int arp_handle_request(arp_t* arp, const char* inputDevice)
{
    IpAddress ip = arp->target_ip;

    ArpCacheEntry* arpCacheEntry = _arp_cache.get(ip);
    if (nullptr == arpCacheEntry) {
        kprintf("arp: target ip (%08x) not found.\n", ip);
        return 1;
    }

    NetworkBuffer* replyPacket = ethernet_alloc_nbuf(inputDevice, ETH_P_ARP, sizeof(arp_t));
    if (!replyPacket) {
        return 1;
    }

    arp_t* replyArp = arp_hdr(replyPacket);

    replyArp->hardware_type = htons(ARP_HW_ETHERNET);
    replyArp->protocol_type = htons(ETH_P_IPv4);
    replyArp->hardware_length = sizeof(MacAddress);
    replyArp->protocol_length = sizeof(IpAddress);
    replyArp->operation = htons(ARP_OP_RESPONSE);
    memcpy(replyArp->sender_mac, arpCacheEntry->mac, sizeof(MacAddress));
    replyArp->sender_ip = arpCacheEntry->ip;
    memcpy(replyArp->target_mac, arp->sender_mac, sizeof(MacAddress));
    replyArp->target_ip = arp->sender_ip;

    ethernet_send_packet(replyPacket, arp->sender_mac);
    return 0;
}

static MacAddress broadcastMac{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static int arp_send_request(IpAddress ipAddress)
{
    route_t route{};
    MacAddress deviceMac{};

    if (0 != ip_route_lookup(ipAddress, &route)) {
        return -1;
    }

    if (!ethernet_device_hwaddr(route.iface->device, deviceMac)) {
        return -1;
    }

    NetworkBuffer* nbuf = ethernet_alloc_nbuf(route.iface->device, ETH_P_ARP, sizeof(arp_t));
    if (!nbuf) {
        return -1;
    }

    arp_t* request = arp_hdr(nbuf);

    request->hardware_type = htons(ARP_HW_ETHERNET);
    request->protocol_type = htons(ETH_P_IPv4);
    request->hardware_length = sizeof(MacAddress);
    request->protocol_length = sizeof(IpAddress);
    request->operation = htons(ARP_OP_REQUEST);
    memcpy(request->sender_mac, deviceMac, sizeof(MacAddress));
    request->sender_ip = htonl(route.iface->ipv4.address);
    memset(request->target_mac, 0, sizeof(request->target_mac));
    request->target_ip = htonl(ipAddress);

    ethernet_send_packet(nbuf, broadcastMac);
    return 0;
}

static bool arp_is_valid(arp_t* arp) {
    if (ARP_HW_ETHERNET != arp->hardware_type) {
        kprintf("validatePacket - not ether hw type\n");
        return false;
    }

    if (ETH_P_IPv4 != arp->protocol_type) {
        kprintf("validatePacket - not ipv4 proto type\n");
        return false;
    }

    if (sizeof(MacAddress) != arp->hardware_length) {
        kprintf("validatePacket - hw len != 6\n");
        return false;
    }

    if (sizeof(IpAddress) != arp->protocol_length) {
        kprintf("validatePacket - proto len !=4\n");
        return false;
    }

    const auto opMode = arp->operation;
    if (ARP_OP_REQUEST != opMode && ARP_OP_RESPONSE != opMode) {
        kprintf("validatePacket - not a req or a resp\n");
        return false;
    }

    return true;
}

static timeout_callback_ret arp_timeout_callback(void* scheduler, void* arg)
{
    lock_guard<InterruptsMutex> guard(_mutex);
    _arp_cache.iterate([](any_t, char * key, any_t data) -> int {
        ArpCacheEntry* entry = (ArpCacheEntry *) data;
        if (entry->is_static) {
            return MAP_OK;
        }

        if (0 >= (--entry->timeout_seconds)) {
            _arp_cache.remove(entry->ip);
        }

        return MAP_OK;

    }, NULL);

    return TIMER_KEEP_GOING;
}

int arp_set_entry(const char *devName, MacAddress macAddress, IpAddress ipAddress)
{
    lock_guard<InterruptsMutex> guard(_mutex);
    ArpCacheEntry* arpCacheEntry = _arp_cache.get(ipAddress);
    if (nullptr != arpCacheEntry) {
        if (arpCacheEntry->is_static) {
            return 0;
        }

        arpCacheEntry->timeout_seconds = 300;
        strncpy(arpCacheEntry->device, devName, sizeof(arpCacheEntry->device) - 1);
        memcpy(arpCacheEntry->mac, macAddress, sizeof(MacAddress));
        return 0;
    }

    ArpCacheEntry newEntry;
    newEntry.ip = ipAddress;
    newEntry.is_static = false;
    newEntry.timeout_seconds = 300;
    strncpy(newEntry.device, devName, sizeof(newEntry.device) - 1);
    memcpy(newEntry.mac, macAddress, sizeof(MacAddress));

    _arp_cache.put(ipAddress, std::move(newEntry));
    return 0;
}

int arp_delete_entry(IpAddress ipAddress)
{
    return _arp_cache.remove(ipAddress) ?  1 : 0;
}

int arp_set_static(IpAddress ipAddress) {
    ArpCacheEntry* arpCacheEntry = _arp_cache.get(ipAddress);
    if (nullptr == arpCacheEntry) {
        return -1;
    }

    arpCacheEntry->is_static = true;
    return 0;
}

void arp_print_cache(void)
{
    _arp_cache.iterate([](any_t, char * key, any_t data) -> int {
        ArpCacheEntry& value = *(ArpCacheEntry *) data;
        kprintf("IP: %08x MAC: " MAC_FORMAT " STATIC: %s TIME: %d DEV: %s\n",
                value.ip, MAC_ARGUMENT(value.mac), value.is_static ? "true" : "false", value.timeout_seconds, value.device);
        return MAP_OK;
    }, NULL);
}

int arp_get_hwaddr(IpAddress ipAddress, uint8_t *outputMac)
{
    ArpCacheEntry* arpCacheEntry = _arp_cache.get(ipAddress);
    if (nullptr == arpCacheEntry) {
        return -1;
    }

    memcpy(outputMac, arpCacheEntry->mac, sizeof(MacAddress));
    return 0;
}

struct ArpResolveInternalContext {
    IpAddress ipAddress;
    int rounds;
    arp_resolve_cb_t cb;
    void* ctx;
};

static timeout_callback_ret arp_resolve_callback(void* scheduler, NetworkBuffer* arpContext)
{
    MacAddress mac{};
    int found = 0;
    ArpResolveInternalContext* ctx = (ArpResolveInternalContext*) nbuf_data(arpContext);

    if (0 == arp_get_hwaddr(ctx->ipAddress, mac)) {
        found = 1;
        goto exit;
    }

    ctx->rounds--;
    if (0 >= ctx->rounds) {
        found = 0;
        goto exit;
    }

    if ((ctx->rounds % 2) == 0) {
        arp_send_request(ctx->ipAddress);
    }

    return TIMER_KEEP_GOING;

exit:
    if (ctx->cb) {
        ctx->cb(ctx->ctx, found);
    }

    nbuf_free(arpContext);
    return TIMER_THATS_ENOUGH;
}

int arp_resolve(IpAddress ipAddress, arp_resolve_cb_t cb, void* ctx)
{
    MacAddress mac{};
    if (0 == arp_get_hwaddr(ipAddress, mac)) {
        cb(ctx, 1);
        return 0;
    }

    NetworkBuffer* nbuf = nbuf_alloc(sizeof(ArpResolveInternalContext));
    if (!nbuf) {
        return -1;
    }

    ArpResolveInternalContext* arpContext = (ArpResolveInternalContext*) nbuf_data(nbuf);
    arpContext->rounds = 10;
    arpContext->ctx = ctx;
    arpContext->cb = cb;
    arpContext->ipAddress = ipAddress;

    if (0 != arp_send_request(ipAddress)) {
        nbuf_free(nbuf);
        return -1;
    }

    scheduler_set_timeout(100, (timeout_callback_t)arp_resolve_callback, nbuf);
    return 0;
}
