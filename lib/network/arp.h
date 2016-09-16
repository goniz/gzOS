#ifndef GZOS_ARP_H
#define GZOS_ARP_H

#include "ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t IpAddress;

// NOTE: IPv4 over Ethernet specific ARP structure..
typedef struct {
    uint16_t hardware_type;
    uint16_t protocol_type;
    uint8_t hardware_length;
    uint8_t protocol_length;
    uint16_t operation;
    MacAddress sender_mac;
    IpAddress sender_ip;
    MacAddress target_mac;
    IpAddress target_ip;
} __attribute__((packed)) arp_t;

typedef enum {
    ARP_HW_ETHERNET = 1
} arp_hardware_type_t;

typedef enum {
    ARP_OP_REQUEST = 1,
    ARP_OP_RESPONSE = 2
} arp_op_t;

int arp_set_entry(const char *devName, MacAddress macAddress, IpAddress ipAddress);
int arp_set_static(IpAddress ipAddress);
int arp_delete_entry(IpAddress ipAddress);
void arp_print_cache(void);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_ARP_H
