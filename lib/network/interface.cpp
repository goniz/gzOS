#include <vector>
#include "interface.h"
#include "route.h"
#include "arp.h"

static std::vector<interface_t> gInterfaces;

static const interface_t* find_interface(const char* device) {
    for (const interface_t& iface : gInterfaces) {
        if (0 == strcmp(device, iface.device)) {
            return &iface;
        }
    }

    return nullptr;
}

int interface_add(const char *device, IpAddress ip, IpAddress netmask)
{
    interface_t newIface;
    MacAddress deviceMac{};
    const interface_t* iface = find_interface(device);
    if (iface) {
        return -1;
    }

    ethernet_device_hwaddr(device, deviceMac);

    memset(&newIface, 0, sizeof(newIface));
    strncpy(newIface.device, device, sizeof(newIface.device) - 1);
    newIface.ipv4.address = ip;
    newIface.ipv4.netmask = netmask;
    gInterfaces.push_back(newIface);

    ip_route_add(ip & netmask, netmask, 0, &(*gInterfaces.end()));
    arp_set_entry(device, deviceMac, ip);
    arp_set_static(ip);
    return 0;
}

const interface_t* interface_get(const char *device)
{
    return find_interface(device);
}

