#include <platform/drivers.h>
#include <vector>
#include <cstring>
#include "lib/network/ip/ip.h"
#include "lib/network/route.h"

static int ip_route_init(void);
DECLARE_DRIVER(ip_route, ip_route_init, STAGE_FIRST);

static std::vector<Route> _rtable;

int ip_route_lookup(IpAddress destinationAddr, route_t* outputRoute)
{
    for (const Route& route : _rtable)
    {
        assert(NULL != route.iface);
        if (is_ip_on_same_network(route.dest_addr, destinationAddr, route.netmask)) {
            memcpy(outputRoute, &route, sizeof(*outputRoute));
            return 0;
        }
    }

    memset(outputRoute, 0, sizeof(*outputRoute));
    return -1;
}

int ip_route_add(IpAddress destinationAddr, IpAddress netmask, IpAddress gateway, interface_t* iface)
{
    _rtable.push_back({destinationAddr, gateway, netmask, iface});
    return 0;
}

int is_ip_on_same_network(IpAddress ip1, IpAddress ip2, uint32_t mask) {
    return (ip1 & mask) == (ip2 & mask);
}

static int ip_route_init(void)
{
    return 0;
}