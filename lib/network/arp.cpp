#include <platform/drivers.h>
#include <cstring>
#include <lib/kernel/scheduler.h>
#include <platform/cpu.h>
#include <lib/primitives/hashmap.h>
#include "arp.h"
#include "ethernet.h"
#include "nbuf.h"

static int arp_layer_init(void);
static timeout_callback_ret arp_timeout_callback(void* arg);
static void arp_handler(void* user_ctx, NetworkBuffer* nbuf);
static bool arp_is_valid(arp_t *arp);
static void arp_handle_request(arp_t *arp, const char *inputDevice);
static void arp_handle_response(arp_t *arp);

DECLARE_DRIVER(arp_layer, arp_layer_init, STAGE_SECOND + 1);

struct ArpCacheEntry {
    MacAddress mac = {0};
    IpAddress ip = 0;
    char device[16] = {0};

    int timeout_seconds = 300;
    bool is_static = false;
};

static HashMap<IpAddress, ArpCacheEntry> _arp_cache;

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
        return;
    }

    if (ARP_OP_REQUEST == arp->operation) {
        arp_handle_request(arp, inputDevice);
    } else {
        arp_handle_response(arp);
    }

    nbuf_free(nbuf);
}

static void arp_handle_response(arp_t *arp)
{

}

static void arp_handle_request(arp_t* arp, const char* inputDevice)
{
    IpAddress ip = arp->target_ip;

    ArpCacheEntry* arpCacheEntry = _arp_cache.get(ip);
    if (nullptr == arpCacheEntry) {
        kprintf("arp: target ip (%08x) not found.\n", ip);
        return;
    }

    NetworkBuffer* replyPacket = ethernet_alloc_nbuf(inputDevice, ETH_P_ARP, sizeof(arp_t));
    if (!replyPacket) {
        return;
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

static timeout_callback_ret arp_timeout_callback(void* arg)
{
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

//    for (auto iter = _arp_cache.begin(); iter != _arp_cache.end(); ) {
//        auto& entry = iter->second;
//        if (entry.is_static) {
//            iter++;
//            continue;
//        }
//
//        if (0 >= (--entry.timeout_seconds)) {
//            iter = _arp_cache.erase(iter);
//        } else {
//            iter++;
//        }
//    }

    return TIMER_KEEP_GOING;
}

int arp_set_entry(const char *devName, MacAddress macAddress, IpAddress ipAddress) {
    ArpCacheEntry arpCacheEntry;

    arpCacheEntry.ip = ipAddress;
    strncpy(arpCacheEntry.device, devName, sizeof(arpCacheEntry.device) - 1);
    memcpy(arpCacheEntry.mac, macAddress, sizeof(MacAddress));

    _arp_cache.put(ipAddress, std::move(arpCacheEntry));
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
