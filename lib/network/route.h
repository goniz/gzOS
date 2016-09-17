#ifndef GZOS_ROUTE_H
#define GZOS_ROUTE_H

#include "ip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Route {
    char device[16];
    IpAddress dest_addr;
    IpAddress gateway_addr;
    IpAddress netmask;
} route_t;

int ip_route_lookup(IpAddress destinationAddr, route_t* outputRoute);

#ifdef __cplusplus
}
#endif
#endif //GZOS_ROUTE_H
