#include <platform/drivers.h>
#include <vector>
#include <cstring>
#include "ip.h"
#include "route.h"

static int ip_route_init(void);
static bool on_same_network(IpAddress ip1, IpAddress ip2, uint32_t mask);
DECLARE_DRIVER(ip_route, ip_route_init, STAGE_FIRST);

static std::vector<Route> _rtable;

int ip_route_lookup(IpAddress destinationAddr, route_t* outputRoute)
{
    for (const Route& route : _rtable)
    {
        if (on_same_network(route.dest_addr, destinationAddr, route.netmask)) {
            memcpy(outputRoute, &route, sizeof(*outputRoute));
            return 0;
        }
    }

    memset(outputRoute, 0, sizeof(*outputRoute));
    return -1;
}

static bool on_same_network(IpAddress ip1, IpAddress ip2, uint32_t mask) {
    return (ip1 & mask) == (ip2 & mask);
}

static int ip_route_init(void)
{
    return 0;
}