#ifndef GZOS_INTERFACE_H
#define GZOS_INTERFACE_H

#include "ip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char device[16];

    struct {
        IpAddress address;
        IpAddress netmask;
    } ipv4;

} interface_t;

int interface_add(const char* device, IpAddress ip, IpAddress netmask);
const interface_t* interface_get(const char* device);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_INTERFACE_H
