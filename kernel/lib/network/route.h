#ifndef GZOS_ROUTE_H
#define GZOS_ROUTE_H

#include "ip.h"
#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Route {
    IpAddress dest_addr;
    IpAddress gateway_addr;
    IpAddress netmask;
    interface_t* iface;
} route_t;

int ip_route_lookup(IpAddress destinationAddr, route_t* outputRoute);
int ip_route_add(IpAddress destinationAddr, IpAddress netmask, IpAddress gateway, interface_t* iface);
int is_ip_on_same_network(IpAddress ip1, IpAddress ip2, uint32_t mask);

#ifdef __cplusplus
}
#endif
#endif //GZOS_ROUTE_H
